// See LICENSE.SiFive for license details.
// See LICENSE.Berkeley for license details.

#include "verilated.h"
#if VM_TRACE
#include <memory>
#include "verilated_vcd_c.h"
#endif
#include <fesvr/dtm.h>
#include <iostream>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include "VTestHarness.h"

// For option parsing, which is split across this file, Verilog, and
// FESVR's HTIF, a few external files must be pulled in. The list of
// files and what they provide is enumerated:
//
// $RISCV/include/fesvr/htif.h:
//   defines:
//     - HTIF_USAGE_OPTIONS
//     - HTIF_LONG_OPTIONS_OPTIND
//     - HTIF_LONG_OPTIONS
// $(ROCKETCHIP_DIR)/generated-src(-debug)?/$(CONFIG).plusArgs:
//   defines:
//     - PLUSARG_USAGE_OPTIONS
//   variables:
//     - static const char * verilog_plusargs

extern dtm_t* dtm;
static uint64_t trace_count = 0;
bool verbose;
bool done_reset;

// void tick_dtm(VTestHarness *tile, bool done_reset, dtm_t::resp resp_bits, uint32_t req_ready, uint32_t resp_valid) {
//   if (done_reset) {
//     dtm->tick(req_ready,
//               resp_valid,
//               resp_bits);

//     dtm_t::req req_bits = dtm->req_bits();
//     uint32_t resp_ready = dtm->resp_ready();
//     uint32_t req_valid = dtm->req_valid();
//     printf("[send] req_addr: %x req_op: %x req_data: %x resp_ready: %d req_valid: %d\n", req_bits.addr, req_bits.op, req_bits.data, resp_ready, req_valid);

//     tile->io_debug_resp_ready = resp_ready;
//     tile->io_debug_req_valid = req_valid;
//     tile->io_debug_req_bits_addr = req_bits.addr;
//     tile->io_debug_req_bits_op = req_bits.op;
//     tile->io_debug_req_bits_data = req_bits.data;
//   } else {
//     tile->io_debug_req_valid = 0;
//     tile->io_debug_resp_ready = 0;
//   }
// }


void handle_sigterm(int sig)
{
  dtm->stop();
}

double sc_time_stamp()
{
  return trace_count;
}

extern "C" int vpi_get_vlog_info(void* arg)
{
  return 0;
}

static void usage(const char * program_name)
{
  printf("Usage: %s [EMULATOR OPTION]... [VERILOG PLUSARG]... [HOST OPTION]... BINARY [TARGET OPTION]...\n",
         program_name);
  fputs("\
Run a BINARY on the Rocket Chip emulator.\n\
\n\
Mandatory arguments to long options are mandatory for short options too.\n\
\n\
EMULATOR OPTIONS\n\
  -c, --cycle-count        Print the cycle count before exiting\n\
       +cycle-count\n\
  -h, --help               Display this help and exit\n\
  -m, --max-cycles=CYCLES  Kill the emulation after CYCLES\n\
       +max-cycles=CYCLES\n\
  -s, --seed=SEED          Use random number seed SEED\n\
  -r, --rbb-port=PORT      Use PORT for remote bit bang (with OpenOCD and GDB) \n\
                           If not specified, a random port will be chosen\n\
                           automatically.\n\
  -V, --verbose            Enable all Chisel printfs (cycle-by-cycle info)\n\
       +verbose\n\
", stdout);
#if VM_TRACE == 0
  fputs("\
\n\
EMULATOR DEBUG OPTIONS (only supported in debug build -- try `make debug`)\n",
        stdout);
#endif
  fputs("\
  -v, --vcd=FILE,          Write vcd trace to FILE (or '-' for stdout)\n\
  -x, --dump-start=CYCLE   Start VCD tracing at CYCLE\n\
       +dump-start\n\
", stdout);
  fputs("\n" HTIF_USAGE_OPTIONS, stdout);
  printf("\n"
"EXAMPLES\n"
"  - run a bare metal test:\n"
"    %s $RISCV/riscv64-unknown-elf/share/riscv-tests/isa/rv64ui-p-add\n"
"  - run a bare metal test showing cycle-by-cycle information:\n"
"    %s +verbose $RISCV/riscv64-unknown-elf/share/riscv-tests/isa/rv64ui-p-add 2>&1 | spike-dasm\n"
#if VM_TRACE
"  - run a bare metal test to generate a VCD waveform:\n"
"    %s -v rv64ui-p-add.vcd $RISCV/riscv64-unknown-elf/share/riscv-tests/isa/rv64ui-p-add\n"
#endif
"  - run an ELF (you wrote, called 'hello') using the proxy kernel:\n"
"    %s pk hello\n",
         program_name, program_name, program_name
#if VM_TRACE
         , program_name
#endif
         );
}

int main(int argc, char** argv)
{
  unsigned random_seed = (unsigned)time(NULL) ^ (unsigned)getpid();
  uint64_t max_cycles = -1;
  int ret = 0;
  bool print_cycles = false;
  // Port numbers are 16 bit unsigned integers. 
  uint16_t rbb_port = 0;
  char ** htif_argv = NULL;
  int verilog_plusargs_legal = 1;

  while (1) {
    static struct option long_options[] = {
      {"cycle-count", no_argument,       0, 'c' },
      {"help",        no_argument,       0, 'h' },
      {"max-cycles",  required_argument, 0, 'm' },
      {"seed",        required_argument, 0, 's' },
      {"rbb-port",    required_argument, 0, 'r' },
      {"verbose",     no_argument,       0, 'V' },
      HTIF_LONG_OPTIONS
    };
    int option_index = 0;
    int c = getopt_long(argc, argv, "-chm:s:V", long_options, &option_index);
    if (c == -1) break;
 retry:
    switch (c) {
      // Process long and short EMULATOR options
      case '?': usage(argv[0]);             return 1;
      case 'c': print_cycles = true;        break;
      case 'h': usage(argv[0]);             return 0;
      case 'm': max_cycles = atoll(optarg); break;
      case 's': random_seed = atoi(optarg); break;
      case 'r': rbb_port = atoi(optarg);    break;
      case 'V': verbose = true;             break;

      // Process legacy '+' EMULATOR arguments by replacing them with
      // their getopt equivalents
      case 1: {
        std::string arg = optarg;
        if (arg.substr(0, 1) != "+") {
          optind--;
          goto done_processing;
        }
        if (arg == "+verbose")
          c = 'V';
        else if (arg.substr(0, 12) == "+max-cycles=") {
          c = 'm';
          optarg = optarg+12;
        }
        else if (arg.substr(0, 12) == "+cycle-count")
          c = 'c';
        // If we don't find a legacy '+' EMULATOR argument, it still could be
        // a VERILOG_PLUSARG and not an error.
        // If we STILL don't find a legacy '+' argument, it still could be
        // an HTIF (HOST) argument and not an error. If this is the case, then
        // we're done processing EMULATOR and VERILOG arguments.
        else {
          static struct option htif_long_options [] = { HTIF_LONG_OPTIONS };
          struct option * htif_option = &htif_long_options[0];
          while (htif_option->name) {
            if (arg.substr(1, strlen(htif_option->name)) == htif_option->name) {
              optind--;
              goto done_processing;
            }
            htif_option++;
          }
          std::cerr << argv[0] << ": invalid plus-arg (Verilog or HTIF) \""
                    << arg << "\"\n";
          c = '?';
        }
        goto retry;
      }
      case 'P': break; // Nothing to do here, Verilog PlusArg
      // Realize that we've hit HTIF (HOST) arguments or error out
      default:
        if (c >= HTIF_LONG_OPTIONS_OPTIND) {
          optind--;
          goto done_processing;
        }
        c = '?';
        goto retry;
    }
  }

done_processing:
  if (optind == argc) {
    std::cerr << "No binary specified for emulator\n";
    usage(argv[0]);
    return 1;
  }
  int htif_argc = 1 + argc - optind;
  htif_argv = (char **) malloc((htif_argc) * sizeof (char *));
  htif_argv[0] = argv[0];
  for (int i = 1; optind < argc;) htif_argv[i++] = argv[optind++];

  if (verbose)
    fprintf(stderr, "using random seed %u\n", random_seed);

  srand(random_seed);
  srand48(random_seed);

  Verilated::commandArgs(argc, argv);
  VTestHarness *tile = new VTestHarness;

#if VM_TRACE
  Verilated::traceEverOn(true); // Verilator must compute traced signals
  std::unique_ptr<VerilatedVcdFILE> vcdfd(new VerilatedVcdFILE(vcdfile));
  std::unique_ptr<VerilatedVcdC> tfp(new VerilatedVcdC(vcdfd.get()));
  if (vcdfile) {
    tile->trace(tfp.get(), 99);  // Trace 99 levels of hierarchy
    tfp->open("");
  }
#endif

  dtm = new dtm_t(htif_argc, htif_argv);

  signal(SIGTERM, handle_sigterm);

  // The initial block in AsyncResetReg is either racy or is not handled
  // correctly by Verilator when the reset signal isn't a top-level pin.
  // So guarantee that all the AsyncResetRegs will see a rising edge of
  // the reset signal instead of relying on the initial block.
  int async_reset_cycles = 2;

  // Rocket-chip requires synchronous reset to be asserted for several cycles.
  int sync_reset_cycles = 10;
  dtm_t::resp resp_bits;
  uint32_t req_ready, resp_valid;
  bool first_cycle_after_reset = true; 

  for (int i = 0; i < 10; i++) {
    tile->reset = 1;
    tile->clock = 0;
    tile->eval();
    tile->clock = 1;
    tile->eval();
    tile->reset = 0;
  }
  done_reset = true;

  while (trace_count < max_cycles) {
    if (done_reset && (dtm->done()))
      break;
    // tile->reset = trace_count < (async_reset_cycles + sync_reset_cycles) * 2; 
    // done_reset = !tile->reset;

    tile->clock = 0;
    tile->eval();
    tile->clock = 1;
    tile->eval();
    // if (resp_bits.data != 0 || resp_bits.resp != 0) {
    //   printf("[receive] data: %x resp: %x req_ready: %d resp_valid: %d\n", resp_bits.data, resp_bits.resp, req_ready, resp_valid);
    // }
    // printf("[receive] data: %x resp: %x req_ready: %d resp_valid: %d\n", resp_bits.data, resp_bits.resp, req_ready, resp_valid);
    trace_count++;
  }

  if (dtm->exit_code())
  {
    fprintf(stderr, "*** FAILED *** via dtm (code = %d, seed %d) after %ld cycles\n", dtm->exit_code(), random_seed, trace_count);
    ret = dtm->exit_code();
  }
  else if (trace_count == max_cycles)
  {
    fprintf(stderr, "*** FAILED *** via trace_count (timeout, seed %d) after %ld cycles\n", random_seed, trace_count);
    ret = 2;
  }
  else if (verbose || print_cycles)
  {
    fprintf(stderr, "*** PASSED *** Completed after %ld cycles\n", trace_count);
  }

  // if (dtm) delete dtm;
  if (tile) delete tile;
  if (htif_argv) free(htif_argv);
  return ret;
}