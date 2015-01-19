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

### Limitations and Bugs

This project is only able to run executables located in places such as `/bin` or `usr/bin/` like `ls`, but not some bash commands such as `cd`.
Look for this feature to be coming soon!

Also, this project is currently under construction and does not work yet.

If you find any bugs in this project, create an issue on GitHub, or, if you want, fix it yourself and issue a pull request.

Have fun and enjoy!
