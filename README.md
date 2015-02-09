# rshell -- Command Shell Project for UCR's CS100

This project is the quarter assignment for UCR's CS100 class.
It contains two programs: `rshell` and `ls`.

### rshell

`rshell` is a command shell that is able to execute commands such as `ls` and `echo`.  It prints the command prompt `$` and then reads in commands in the following form:
```
cmd         = executable [ argumentList ] [ connector cmd ]
connector   = || or && or ;
```
where `executable` is an executable program in the `PATH` and `argumentList` is an optional list of zero or more arguments for the executable that are separated by spaces.
Here, `;`, `||`, and `&&` work like they do in bash:

1. `;` will always execute the command after it.

2. `&&` will execute the next command only if the previous command succeeded.

3. `||` will execute the next command only if the previous command failed.

For example, if you type in the following command:
```
$ ls -a; echo hello && mkdir test || echo world
```
then rshell will execute the `ls` command to print files in the current directory with the `-a` flag.  
Then it will print to the terminal `hello`, and if that command succeeds, it will make a new directory called `test` with the `mkdir` command.
If either the `echo hello` or `mkdir test` commands failed, then the `echo world` command will be executed.

Typing the connectors `&&` and `||` at very end of command will result in error.

rshell also allows for the use of comments by typing in the `#` character.
Anything after `#` on a line will be ignored.

### ls

`ls` is a program that lists the files in the current directory.
By passing in files and directories as arguments to `ls`, you can also get the contents of a specific directory.

`ls` also supports the use of the `-a`, `-R`, and `-l` flags.

1. `-a` will show the hidden files in a directory.

2. `-R' will recursively list the contents of a directory and its subdirectories.

3. `-l` will show the [long listing](http://forum.linuxcareer.com/threads/1659-ls-command-with-a-long-listing-format-output) format of the files in a directory.

The `ls` program lastly will output directories in blue text, executables in green text, and hidden files with a gray background.

### Installing and Running rshell and ls

After cloning and downloading this repository, `cd` into it and type `make` in the command prompt. This will then make `rshell` and `ls`  for you (requires the `g++` compiler to be installed on your system).
If you wish to only make `rshell` or only `ls`, type `make rshell` and `make ls` respectively.

To run `rshell`, type
```
$ bin/rshell
```
and this will launch the rshell command prompt.

To exit rshell, just type `exit`.

To run `ls`, type
```
$ bin/ls [args and flags]
```
and this will run `ls`.
This also works while running `rshell`.

 Enjoy!

### Bugs

The program `rshell` is only able to run executables located in `/bin` or `usr/bin/` like `ls`, or in filepath the user specifies, but not common bash commands such as `cd`.
Look for this feature to be coming soon!

Other bugs in rshell:

1. Error messages for syntax errors with connectors are too general.

2. Commands typed before a `;` will run when not supposed to when certain syntax error occurs with `&&` or `||`.

`ls` has some bugs as well:

1. If you uncomment line 90 of `src/ls.cpp`, this causes the total block count to be displayed.
The number shown is incorrect. Files get counted double the normal block count, and symbolic links add a number to the total block count, when their individual block count should be zero.

2. This program cannot correctly identify symbolic links for the long listing format.
It also displays the info for the file it is linked to, rather than itself.

3. If passed in a single bad file, `ls` defaults to printing files in current directory rather than just quitting.

If you find any bugs in this project, create an issue on GitHub, or, if you want, fix it yourself and issue a pull request.

Have fun and enjoy!
