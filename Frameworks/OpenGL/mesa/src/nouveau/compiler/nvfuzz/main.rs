/*
 * Copyright © 2022 Collabora, Ltd.
 * SPDX-License-Identifier: MIT
 */

extern crate bitview;

use crate::bitview::*;

use std::fs;
use std::io::Write;
use std::ops::Range;
use std::path::PathBuf;
use std::process::Command;

const TMP_FILE: &str = "/tmp/nvfuzz";

fn find_cuda() -> std::io::Result<PathBuf> {
    let paths = fs::read_dir("/usr/local")?;

    for path in paths {
        let mut path = path?.path();
        let Some(fname) = path.file_name() else {
            continue;
        };

        let Some(fname) = fname.to_str() else {
            continue;
        };

        if !fname.starts_with("cuda-") {
            continue;
        }

        path.push("bin");
        path.push("nvdisasm");
        if path.exists() {
            return Ok(path);
        }
    }

    Err(std::io::Error::new(
        std::io::ErrorKind::NotFound,
        "Failed to find nvdisasm",
    ))
}

//fn write_tmpfile(data: &[u32]) -> std::io::Result<()> {
//    let mut file = std::fs::File::create(TMP_FILE)?;
//    for dw in data {
//        file.write(dw.to_le_bytes())?;
//    }
//}

fn main() {
    let args: Vec<String> = std::env::args().collect();
    let sm: u8 = {
        let sm_str = &args[1];
        assert!(sm_str.starts_with("SM"));
        sm_str[2..].parse().unwrap()
    };
    let range: Vec<&str> = args[2].split("..").collect();
    let range: Range<usize> = Range {
        start: range[0].parse().unwrap(),
        end: range[1].parse().unwrap(),
    };

    let dw_count = if sm >= 70 {
        4
    } else if sm >= 50 {
        8
    } else {
        panic!("Unknown shader model");
    };

    let mut instr = Vec::new();
    for i in 0..dw_count {
        instr.push(u32::from_str_radix(&args[3 + i], 16).unwrap());
    }

    let cuda_path = find_cuda().expect("Failed to find CUDA");

    for bits in 0..(1_u64 << range.len()) {
        BitMutView::new(&mut instr[..]).set_field(range.clone(), bits);

        print!("With {:#x} in {}..{}:", bits, range.start, range.end);
        for dw in &instr {
            print!(" {:#x}", dw);
        }
        print!("\n");

        let mut data = Vec::new();
        for dw in &instr {
            data.extend(dw.to_le_bytes());
        }
        std::fs::write(TMP_FILE, data).expect("Failed to write file");

        let out = Command::new(cuda_path.as_path())
            .arg("-b")
            .arg(format!("SM{sm}"))
            .arg(TMP_FILE)
            .output()
            .expect("failed to execute process");
        std::io::stderr().write_all(&out.stderr).expect("IO error");
        std::io::stdout().write_all(&out.stdout).expect("IO error");
    }
}
