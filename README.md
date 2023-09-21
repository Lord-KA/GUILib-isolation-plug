# GUILib isolation plugin
This is an isolation plug-in that utilizes Qubes qrexec to run untrusted
standard ([Elpidifor-s-legacy](https://github.com/mishaglik/Elpidifor-s-legacy))
plug-ins inside a different VM in a fully transparent way.
## Server
The binary that is being run inside an untrusted VM is called the `server`.
It emulates all interfaces that should be provided by [standard](https://github.com/mishaglik/Elpidifor-s-legacy)
Photoshop. The server loads untrusted plug-ins (dynamic libraries) that were transferred to
the VM using QubesOS tools. It registers them in the main application via [RPCs](https://github.com/Lord-KA/GUILib-isolation-plug/blob/master/shared/proxy-event.hpp).
Then the RPC processing loop is initiated. It handles all client requests to plug-ins.

## Client
The dynamic library that is basically a complex standard plug-in itself is called the `client`.
It detects all untrusted plug-ins in the `UntrustedPlugins` directory, copies them to untrusted
VM and initiates connection with the server. After all of the plug-ins are registered in the Photoshop,
the `client` handles user actions and Photoshop events by sending requests to the server. For now, there
is no formal connection closed event and the untrusted VM isn't being killed afterwards, but that is in
the TODO list.

## Shared
The header describing RPC is shared between the client and the server.

## Known issues and limitations (treat as TODO list)
* There is no real data verification on the client side. If the Photoshop-provided standard functions
  are lacking these checks too, malicious plug-in may perform e.g. a buffer overflow attack via `put/getPixel()`.
  It seems quite unlikely to cause real trouble, but no reason to skip verification, so **TODO**.
* There is almost no durability in the primitive RPC. If one of the calls happens at the wrong time,
  or the channel changes the order of calls, the client or the server will assert. While this is still
  a qrexec isolation plug-in, it may not be an issue, as the calls sequencing is finessed and the channel
  is a perfect link. It's not that hard to use network communications instead, so it would be nice to have
  some fallback. As the server is an implicit state machine, it wouldn't be hard to implement, e.g. if a
  function in apply fails, just start a new apply cycle. So, **TODO**.
* It can be used as a limited side channel between untrusted and trusted VMs. By design, the plug-in can
  read and write data to the common buffer. If both server and client are compromised, they can just use
  the qrexec or Xen channels and get a high-bandwidth channel. The attack requires compromise of the trusted
  VM, so it is GameOver<sup>tm</sup> anyway.
* The setup requires configuration inside dom0. This is a QubesOS security feature, so nothing can be done
  really. Same goes for the packaging. The service on untrusted VM has to be set up by hand and I am not
  going to provide a Qubes-approved package to be installed in dom0 for it to configure everything.
* You tell me in the [issues](https://github.com/Lord-KA/GUILib-isolation-plug/issues).
