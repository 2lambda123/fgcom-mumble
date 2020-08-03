FGCom-mumble - eine (Flugsimulator-) Funksimulation basierend auf Mumble
========================================================================

Dies ist die deutsche Übersetzung der serverseitigen Details.  
Die englische Version ist stets führend und die deutsche Version enthält eventuell nicht den aktuellsten Stand!

([-> english original version](Readme.server.md))

Installation / Setup für den Serverteil
=======================================

Voraussetzungen
---------------
- Standard Mumble Murmur Server Instanz mit Pluginsupport, >= v1.4.0.

Das ist bereits genug, um einen einfachen FGCom-mumble Server bereitzustellen, der Funkkommunikation zwischen den Teilnehmern ermöglicht.
Die *fgcom-mumble* Plugins der einzelnen Teilnehmer kümmern sich um den Rest.

Weitere Funktionalität, wie ATIS-Aufzeichnungen, werden durch einige serverseitige Mumblebots bereitgestellt. Um diese auszuführen, werden einige Pakete benötigt:

- ein luajit 5.1 Interpreter (`apt-get install luajit lua5.1 lua-bitop`)
  - lua mumble support (*mumble.so*) im Lua include Pfad (/usr/lib/x86_64-linux-gnu/lua/5.1/; kompiliert von [https://github.com/bkacjios/lua-mumble])


Einen Server betreiben
----------------------
- Der Mumbleserver muss laufen
- Erstelle einen Kanal, der `fgcom-mumble` heißt. Das Plugin arbeitet nur für Teilnehmer in diesem speziellen Kanal.
- Teilnehmer verbinden sich auf den Server und aktivieren ihr FGCom Plugin. Dieses kümmert sich um sämtlichen Funkverkehr auf allen Frequenzen.
- Erzeuge Zertifikate für die Bots (details siehe unten)
- Starte den `fgcom-bot-manager`, der sich um die benötigten Bots kümmert: `./fgcom-botmanager.sh`



Der Botmanager
==================
`./fgcom-botmanager.sh --help` zeigt Aufrufinformationen an.

Der Botmanager startet selbständig die benötigten Bots.  
Er erzeugt außerdem die notwendie Interprozesskommunikation für den Aufnahmebot. Danach startet er den Aufnahmebot und wartet auf Nachrichten von ihm. Sobald eine aunahmeinformation eintrifft, startet der Botmanager einen passenden Wiedergabebot.

Der Statusbot kann eine URL für die Statuswebseite in seinem Kommentar ankündigen. Um dies zu aktivieren: `./fgcom-botmanager.sh --sweb=<url>`.


Aufnahmebot
===================
Wenn du den Aufnahmebot selbst manuell aufrufen möchtest, kannst du seine Optionen mit `luajit fgcom-radio-recorder.bot.lua -h` ansehen.

Der Aufnahmebot überwacht den `fgcom-mumble` Kanal für Aufnahmeanfragen durch Teilnehmer. Wenn so eine Anfrage erkannt wird, nimmt er die Tonspur auf und speichert sie in einer FGCS-Datei auf die Festplatte.
Diese FGCS-Datei kann vom Wiedergabebot aufgenommen und abgespielt werden, was normalerweise durch den Botmanager automatisch passiert.


Anfrage zur Aufnahme (z.B. ATIS)
--------------------------------
Eine ATIS-Aufnahmeanfrage ist eine ganz normale Übertragung, allerdings auf einer speziellen "Frequenz" im Format `RECORD_<target-frequency>`.
Sobald ein Teilnehmer auf einer solchen Frequenz überträgt, wird die Aufnahme aufgezeichnet. sobald die Aufnahme endet, zeichnet der Recorder die
Position, Stärke des Signals und das Rufzeichen auf und speichert alles zusammen in einer FGCS-Datei.
Anschließend informiert der Recorder den Botmanager, dass eine neue Aufzeichnung vorliegt. Dieser startet einen passenden Wiedergabebot, der die Aufnahme mit dem Aufnahmerufzeichen von der entsprechenden Position aus überträgt.
Der Wiedergabebot wird automatisch beendet, wenn seine maximale Gültigkeit abläuft.

Beachte, dass die Aufzeichnungen nicht ATIS-Spezifisch sind. Mit der dargestellten Methode lassen sich beliebige Frequenzen aufzeichnen (z.b. Radiosender).


910.00 Testfrequenz Aufnahmeanfrage
---------------------------------------
Die Testaufnahmen auf der Frequenz 910.00 dienen zum Verifizieren des eigenen Setups.  
Soweit die Spezifikation hier betroffen ist, sind Testaufnahmen ganz normale Aufnahmeanfragen. Allerdings weichen sie in folgenden Punkten ab:

- Aufnahmen geschehen auf der Frequenz `910.00`, d.h. ohne `RECORD_` Präfix.
- Die Wiedergabe der Aufnahme muss ebenfalls auf dieser Frequenz erfolgen.
- Die Wiedergabe muss an der Position der Aufnahme erfolgen.
- Die Wiedergabeart ist "oneshot" (einmalige Wiedergabe) und nur für kurze Dauer gültig. Der Wiedergabebot muss sich nach dem Abspielen selbst beenden.

Diese Anforderungen werden durch den Recorderbot erfüllt, der die Testfrequenz mitüberwacht und die FGCS-Dateiheader entsprechend schreibt.


FGCom Sample file disk format (`.fgcs`)
---------------------------------------
Das FGCS-Dateiformat enthält die aufgezeichneten Soundsamples zusammen mit Metainformationen. Die Dateien sollten einen eindeutigen Namen mit `.fgcs` Postfix haben.  
Der Inhalt beginnt mit einem zeilenweisen, menschenlesbaren Header, gefolgt von den binären Sampledaten.

Das Dateiormat wird im Detail in der [englischen Version](Readme.server.md#fgcom-sample-file-disk-format-fgcs) dargestellt.


Wiedergabebot
==================
Falls du den Bot manuell starten möchtest, kannst du die verfügbaren Optionen mit `luajit fgcom-radio-playback.bot.lua -h` ausgeben lassen.

Der Bot verbindet sich auf den Server und sendet dann seine Position an die verbundenen Teilnehmer. Danach beginnt er, die Sampledaten abzuspielen.
Dies macht er in einer Schleife, bis entweder die Daten nicht mehr gültig sind (abgelaufen oder vom Typ "oneshot"), die Datei von der Platte gelöscht wurde, oder der Bot von außen beendet wird.

Die zum Abspielen benötigten Informationen werden entweder aus dem `fgcs`-Dateiheader gelesen, oder mit dem Kommandozeilenaufruf mitgegeben (letztere werden in diesem Fall bevorzugt).

Der Bot entfernt die FGCS-Datei nach dem Abspielen, wenn sie nicht mehr gültig ist.

Aktuell kann der Bot nur FGCS-Daten abspielen. In der Zukunft könnte er auch OGG-Daten abspielen, diese würden dann wahrscheinlich als "persistent" angenommen werden (um z.B. vordefinierte Radioprogramme abzuspielen).


Bot Zertifikate
=======================
Zum Anmelden am mumble-Server muss jeder Bot sein eigenes Zertifikat benutzen (ansonsten kickt ihn der Server eventuell).  
Diese können einfach folgendermaßen erzeugt werden:
```
for w in rec play status;
  do  openssl genrsa -out ${w}bot.key 2048 2> /dev/null
  openssl req -new -sha256 -key ${w}bot.key -out ${w}bot.csr -subj "/"
  openssl x509 -req -in ${w}bot.csr -signkey ${w}bot.key -out ${w}bot.pem 2> /dev/null
done
```



Serverseitige Komponenten selbst kompilieren
============================================
- Die lua-Bots müssen nicht kompiliert werden, sondern werden über einen Interpreter gestartet. Luajit ist üblicherweise bereits als Programm in gängigen Linuxdistributionen enthalten.
- Den mumbleserver muss man ebenfalls normalerweise nicht selbst kompilieren benutze einfach die Version der Distribution (das gleiche gilt für den mumble-client).

- Informationen, um das Lua mumble-modul *mumble.so* zu kompilieren, gibt es auf [bkacjios Githubseite](https://github.com/bkacjios/lua-mumble).
  Diese Bibliothek muss im lua Bibliotheksverzeichnis abgelegt werden.
- Zusammengefasst für ein Debiansystem:
  - Build dependencys: `apt-get install libluajit-5.1-dev protobuf-c-compiler libprotobuf-c-dev libssl-dev libopus-dev libev-dev`
  - lua mumble lib bauen: `~/lua-mumble$ make all`
  - lua mumble lib installieren: `~/lua-mumble$ cp mumble.so /usr/lib/x86_64-linux-gnu/lua/5.1/`