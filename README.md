# Rockstar-C
Rockstar-C is a Transpiler converting a subset of ANSI C to the [Rockstar Programing language](https://codewithrockstar.com/).

**Currently supported language Features:**

* Functions (incl. return)
* Variables
* while loops
* break / continue
* if directives (currently without else)
* assignment expressions
* Comments

**How to use:**

If you are running Linux, download the RockstarC file and run it using ./RockstarC *Filname*
where *Filename* is the c file you want to transpile.
On other Platforms, you need to Compile the Rockstar.c file yourself, and run it however you run command line programs.

**Output**

RockstarC will output two files for each input file, both with the same name as the input file but with different file extensions. The first one is a .parse which obeys no Standards whatsoever and the second one is a .rock which follows [the Rockstar Language Specification](https://codewithrockstar.com/docs) and should run in [the Online Interpreter](https://codewithrockstar.com/online).
