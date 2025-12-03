from pathlib import Path
from typing import List
import subprocess
from concurrent.futures import ProcessPoolExecutor, wait


def get_emulators(tool: str, design: str, *args, **kwargs):
    base = Path(canonicalize_tool_name(tool)) / f"build-{design}"
    if tool == "repcut":
        return find_repcut_emulators(base, *args, **kwargs)
    elif tool == "essent":
        return find_emulator_variants(base, *args, **kwargs)
    else:
        return default_emulator_path(base)


def find_repcut_emulators(base: Path, par_levels: List[int]) -> List[Path]:
    return find_emulator_variants(base, [f"par-{level}" for level in par_levels])


def find_emulator_variants(base: Path, sublevels: List[str]) -> List[Path]:
    return [base / f"{level}" / "emulator" for level in sublevels]


def default_emulator_path(base: Path):
    return [base / "emulator"]


def canonicalize_tool_name(tool: str) -> str:
    if tool == "repcut" or tool == "essent":
        return tool + "-sim"
    else:
        return tool


def get_workloads(workloads_dir=Path("workloads/benchmarks")) -> List[Path]:
    return [f for f in workloads_dir.iterdir()]


DEFAULT_HARNESS_ARGS = ["-c"]
DEFAULT_MAX_NUM_WORKERS = 8


def prepare_patronus_jit_run_task(design: str):
    base = Path("patronus-sim")
    emulator = base / "patronus" / "resources" / "emulator"
    btor = base / f"build-{design}" / "TestHarness.btor"
    prefix = Path("results") / design / "patronus"
    prefix.mkdir(parents=True, exist_ok=True)
    executor_payloads = []
    for bench in get_workloads():
        executor_payloads.append(
            (
                prefix / f"{bench.name}.out",
                ["time", str(emulator), *DEFAULT_HARNESS_ARGS, btor, str(bench)],
            )
        )
    return [(run_one_bench, *arg) for arg in executor_payloads]


def prepare_bench_run_task(tool_name: str, design: str, *args, **kwargs):
    emulators = get_emulators(tool_name, design, *args, **kwargs)
    workloads = get_workloads()
    if len(emulators) > 1:
        tags = [f.parent.name for f in emulators]
    else:
        tags = [None]
    executor_payloads = []
    for bench in workloads:
        for tag, emulator in zip(tags, emulators):
            prefix = Path("results") / design / tool_name
            if tag is not None:
                prefix = prefix / tag
            prefix.mkdir(parents=True, exist_ok=True)
            executor_payloads.append(
                (
                    prefix / f"{bench.name}.out",
                    ["time", str(emulator), *DEFAULT_HARNESS_ARGS, str(bench)],
                )
            )
    return [(run_one_bench, *arg) for arg in executor_payloads]
    with ProcessPoolExecutor(
        max_workers=min(len(workloads), DEFAULT_MAX_NUM_WORKERS)
    ) as executor:
        tasks.extend(
            [executor.submit(run_one_bench, *arg) for arg in executor_payloads]
        )


def run_one_bench(save_path: str, command: List[str]):
    result = subprocess.run(
        command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
    )
    with open(save_path, "w") as f:
        f.write(result.stdout)
        f.write(result.stderr)


if __name__ == "__main__":
    tasks = [
        *prepare_bench_run_task("repcut", "rocket20", [2, 4]),
        *prepare_bench_run_task("repcut", "boom21", [2, 4]),
        *prepare_bench_run_task("essent", "rocket20", ["O3"]),
        *prepare_bench_run_task("essent", "boom21", ["O3"]),
        *prepare_bench_run_task("verilator", "rocket20"),
        *prepare_bench_run_task("verilator", "boom21"),
        *prepare_patronus_jit_run_task("rocket20"),
        *prepare_patronus_jit_run_task("boom21"),
    ]
    with ProcessPoolExecutor(
        max_workers=min(len(tasks), DEFAULT_MAX_NUM_WORKERS)
    ) as executor:
        futures = [executor.submit(*task) for task in tasks]
        wait(futures)
