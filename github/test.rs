fn main() {
    println!("Hello, world!");
    let mut x = 5;
    if x > 3 {
        x = x + 1;
    }
    for i in 0..10 {
        println!("{}", i);
    }
    match x {
        1 => println!("one"),
        _ => println!("other"),
    }
}