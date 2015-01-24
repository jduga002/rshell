# rshell -- Command Shell Project for UCR's CS100

This program is a command shell that is able to execute commands such as `ls` and `echo`.  It prints the command prompt `$` and then reads in commands in the following form:
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

### Installing and Running rshell

After cloning and downloading the repository, `cd` into it and type `make` in the command prompt. This will then make rshell for you (requires the `g++` compiler to be installed on your system).

To run, type
```
$ bin/rshell
```
and this will launch the rshell command prompt.

To exit rshell, just type `exit`.

 Enjoy!

### Bugs

This project is only able to run executables located in `/bin` or `usr/bin/` like `ls`, or in filepath the user specifies, but not common bash commands such as `cd`.
Look for this feature to be coming soon!

Other bugs:

1. Error messages for syntax errors with connectors are too general.

2. Commands typed before a `;` will run when not supposed to when certain syntax error occurs with `&&` or `||`.

If you find any bugs in this project, create an issue on GitHub, or, if you want, fix it yourself and issue a pull request.

Have fun and enjoy!
