fn main() -> std::io::Result<()> {
    let input_fir = std::env::args().nth(1).expect("input fir file missing");
    let content = std::fs::read_to_string(input_fir)?;
    for line in content.lines().filter(|line| !should_strip(line)) {
        println!("{line}");
    }
    Ok(())
}

fn should_strip(line: &str) -> bool {
    let trimmed = line.trim_start();
    trimmed.starts_with("printf(") || trimmed.starts_with("stop(")
}
