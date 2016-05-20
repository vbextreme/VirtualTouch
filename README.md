VirtualTouch v0.1.1
===================
Virtual touch permette di localizzare la posizione di un oggetto attraverso quattro sensori sr04<br/>
è presente il firmware che permette l'interfacciamento con un PC o l'utilizzo in modalità standalone
Stato:
======
* Firmware + Software (0.0.0)<br/>
* Firmware, add timer0, increase speed serial com (0.1.0)<br/>
* Software, add moore alpha count, union mouse and display (0.1.0)<br/>
* Software, add mouse press and release (0.1.1)<br/>

Bug:
====

Require:
========
Il software può essere usato come interfaccia mouse, in quel caso è richiesta la libreria X11

Build
=====
per compilare senza interfaccia mouse<br/>
$ make app<br/>
<br/>
per compilare con interfaccia mouse<br/>
$ make CL=-lX11 DC=XORG app<br/>
<br/>
in caso di crash improvvisi abilitare le assert:<br/>
$ make DC="-D__DEBUG=1 -D__DEBUG_LEVEL=1 -D__DEBUG_TERMINAL -D__ASSERT=1" app<br/>
