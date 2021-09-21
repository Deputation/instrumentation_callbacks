## instrumentation_callbacks
Instrumentation callbacks are quite a fun undocumented part of Windows.
All the code in this repository is released under the MIT license. This repository uses google style C++.

# What are they?
Instrumentation callbacks will let you handle all syscalls (and exceptions!) dispatched by a process on which you placed a callback. 

# The problem with callbacks
Of course, every syscall will be caught, so while the callback is executing you normally shouldn't issue any more syscalls inside it unless you want to end up recursing forever.

# The solution
One solution to this problem could be setting a flag to true/false that enables and disables the callback's analysis capabilities, essentially letting every syscall through if another syscall is already being handled.

This, however, will let most syscalls seep through.

This repository, instead, solves the issue of not being able to issue syscalls inside a callback by setting a TLS-dependent variable, making sure we handle **all** syscalls from all threads while being able to call them ourselves, the only syscalls we'll miss are the ones we'll issue from inside the callback, which we arguably don't care about.

# Hypothetical uses
Instrumentation callbacks have a lot of possible uses, one of them could be for anti-cheating purposes, you could analyze syscalls coming from the process, and determine whether they come from an illegitimate address space or are being used for nefarious purposes without needing to fill system DLLs with bytepatches and hooks.

You could also use them to gain code execution inside a process, if you can allocate and write to memory then set the process' information, you will be able to hijack a thread executing a syscall by making it call a callback of yours.

Of course, I do not condone the usage of any code in this or my other repositories to develop cheating/malicious software.

What you do with this is your own responsibility.