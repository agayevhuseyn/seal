# What is Seal?

Seal is a high-level, dynamic programming language designed to make programming fun. It has a simple and concise syntax that's easy to read and write.
## Installation

Clone the repo and run "build.sh"

```bash
git clone https://github.com/agayevhuseyn/seal.git
cd seal
./build.sh
./build.sh --windows # for windows build
```

## Seal home page

https://www.seallang.org

## Hello, Seal!

```javascript
print("Hello, Seal!") // output: Hello, Seal!

define factor(n)
    if n > 1
        return n * factor(n - 1)
    return 1
```

## Contributing

Pull requests are welcome. For major changes, please open an issue first
to discuss what you would like to change.

Please make sure to update tests as appropriate.
