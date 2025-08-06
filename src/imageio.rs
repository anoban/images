use std::{fs::File, vec::Vec};

pub fn open(path: &str) -> Vec<u8> {
    let mut fptr = File::open(path);
}

mod tests {}
