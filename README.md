# What is Seal?

Seal is an interpreted dynamically typed programming language. Simplicity, minimal syntax, quick development time with less lines of code are main purposes of the language.
## Installation

Clone the repo and run "build.sh"

```bash
git clone https://github.com/agayevhuseyn/seal.git
cd seal
./build.sh
./build.sh --windows # for windows build
```

## Seal home page

https://agayevhuseyn.github.io/seal-web

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
