Der „PeePoo“
============

Was ist das?
------------

<img src="Natürliches Habitat.jpg" width="50%" title="Der PeePoo in seinem natürlichen Habitat">
Unser Hund sucht viel Kontakt und ist reichlich aktiv. Noch hat er aber keine ausreichende Erziehung um selber rausgehen zu dürfen, mal abgesehen davon, dass das Grundstück zum aktuellen Zeitpunkt noch nicht hundesicher ist.

Die junge Dame gibt natürlich Laut wenn sie mal raus muss, auch nachts kommt das noch vor (ist jetzt 9 Monate, ist also wirklich selten). Nur manchmal hat sie anstatt Druck einfach Kontaktbedürfnis. Das kann tagsüber oft noch gewährleistet werden, aber nachts ist das schon schwieriger. Da wir uns die Arbeit teilen, kann jeder in der Familie, der den Hund hört, mal eben raus – was man aber in der Nacht eigentlich nur machen will, wenn es Not tut. Ein Spielkamerad für nachts wollen wir alle nicht sein.

Woran erkenne ich denn nun, dass der Hund tatsächlich muss und nicht – wie schon öfter passiert – einfach alle 15 Minuten ein anderes Familienmitglied wachquiemt und dann steht man im Schlafanzug vor der Tür und schaut einer Hundedame beim Blumenschnuppern zu. Wenn man jetzt wüsste, dass sie erst vor ein paar Minuten mit einem anderen Familienmitglied schon draußen war… Tada – die Idee des PeePoo-Timers ward geboren. Natürlich im Grunde total unsinnig und übertechnisiert, aber wozu ist man denn Bastler?

Warum eigentlich?
-----------------

Ich suchte also eine Einsatzmöglichkeit für meine neu eingetroffenen Digisparks, kleine, auf einem ATtiny85 basierende Microcontroller Devboards mit wenigen IOs (6 an der Zahl) und einer USB-Schnittstelle (<https://images-wixmp-ed30a86b8c4ca887773594c2.wixmp.com/i/0995f7a6-730b-48d0-8612-c4408d15e84d/dc7h4n3-51e6389c-5f86-4fb2-a7ad-34f75f672003.png>). Ich fand die Dinger ganz schuckelig und hab einfach 5 Stück bestellt.

Der ATtiny85 hat 8kB Flash-Speicher (oder wars EEPROM?) an Board, davon gehen beim Digispark ca. 2 kB für den Bootloader (nennt sich dort „micronucleus“) drauf, den die Büchse für Firmware-Updates (also Programm-Updates) benötigt. Dabei wird der Digispark in die USB-Schnittstelle des Programmier-Rechners (mit Arduino) gesteckt und bildet in den ersten 5 Sekunden eine USB1.1 Schnittstelle in Software ab. Findet kein Zugriff eines Programmers statt (spezielle Software namens „micronucleus“ - also genau wie der Bootloader), schaltet er sich ab und startet das eigentliche Programm (also den „Arduino-Sketch“).

Das macht die Sache schon mal ungewöhnlich denn mit den Hausmitteln ist ein schneller Boot damit ausgeschlossen – unter 5 Sekunden macht er es nicht… Weiteres „Schmankerl“ - ich habe die Hardware Version „rev3“ erhalten. Die gibt es aber nicht vom eigentlichen Hersteller, also ein Klon. Grundsätzlich kein Problem, das Design ist offen, aber Klone haben beim Digispark oft nicht die Fuses des ATtiny wie beim Original gesetzt. Auch ist der Bootloader oft veraltet, so auch bei mir. Problem mit den Fuses: Pin D5 ist ohne richtige Fuses ein Reset-Knopf der beim Programmieren verwendet wird. Ändert man das, ist man bei einem Bootloader-Defekt auf einen High-Voltage-Programmer angewiesen, man muss also abwägen ob man das „Risiko“ fährt oder auf den Pin verzichten kann. Es ist ein A/D Pin, aber ohne hardwareseitige Sonderfunktion wie PWM oder I2C-Schnittstelle. (Details hierzu auch hier: <https://wolles-elektronikkiste.de/digispark-die-bequeme-attiny85-alternative>)

Mist, ein Pin zu wenig
----------------------

Mir hatte der Pin gefehlt, weil ich mit Pin P1, an dem auch die interne LED hängt, so meine Probleme hatte. Außerdem habe ich 5 Stück gekauft, ca. 1 € pro Stück, das tut mir jetzt erstmal nicht weh. Ich habe also die Fuses neu gesetzt (gemäß dieser Anleitung: <http://anleitung.joy-it.net/wp-content/uploads/2016/09/ARD-Digispark-Anleitung-3.pdf>).

Vorher habe ich aber einen neuen Bootloader aufgespielt, den es im Git-Repo (<https://github.com/micronucleus/micronucleus/tree/master>) des Herstellers Digistump gab. (Anleitung hier: <https://frottier.wordpress.com/2017/11/12/kein-frust-mit-digispark-clones/>.) Es gibt auch Firmware die den Bootvorgang verkürzt und nur dann auf den USB-Modus schaltet wenn P0 oder P5 besonders geschaltet werden (Anleitung hier: <https://bytelude.de/2018/04/20/wie-man-das-5-sekunden-boot-delay-beim-digispark-digistump-attiny85-entfernt/>). Bei meinem Anwendungsfall habe ich mich dafür entschieden, dass die Büchse eh immer läuft also kein Bedarf an 5s-Beseitigung. Neuer Bootloader aber sollte drauf, weil das die Kompatibilität und Stabilität (positiv) zu beeinflussen scheint, so die Berichte im Internet. Der Bootloader wird ohne ISP programmiert, indem er als normales Programm installiert wird. Dieses Programm nimmt dann den Code des Bootloaders und überschreibt damit den bisherigen Bootloader. Hab ich so auch noch nicht gesehen.

Nicht Rocket-Science sondern Rocket-Speed
-----------------------------------------

Nachdem ich das Board bei meiner Arduino-Umgebung installiert habe, habe ich das Update schnell noch ausgeführt (über die Linux-Kommandozeile und dem im Arduino-Paket beiliegenden micrornucleus-Tool) und sofort gewundert – das simple Blink-Beispiel wollte nicht übertragen werden. Der Bootloader und das micronucleus Tool haben sich nicht mehr verstanden – immerhin wusste das Tool zu berichten, dass der Bootloader zu neu sei. Also das Repo des Herstellers besorgt, das dortige aktuelle Tool übersetzt und dann über das im Arduino-Paket befindliche kopiert. Nicht schön, funktioniert aber. Das Blink-Beispiel klappt nun auch.

Sehr schön, für mich als ESP-Nutzer, sind die superschnellen Tools zum Übersetzen sowie der Mini-Code der generiert wird. Das ist man als ESP-Nutzer einfach nicht mehr gewohnt. Nicht nur, dass das Übersetzen gefühlte 20 Sekunden dauert (Digispark &lt; 1s), auch das Übertragen der mindestens 250kB dauert ein wenig, das Blink-Beispiel hat keine 900 Byte und war in Bruchteilen auf dem Chip. Den steckt man im Gegensatz zu anderen Programmern übrigens genau dann ein, wenn die Programmer-Software ihn sehen möchte. Also hier auch kein COM-Port Gefrickel, vor allem unter Linux gerne ein Graus, wenn die Nummerierung mal wieder umgeworfen wird…

Der Ablauf ist also Sketch schreiben, Übertragen anklicken, warten auf den Hinweis, das kleine Ding einzustöpseln, das dann auch machen und bäm – fertig ist's. Empfehlenswert ist es aber nicht, den Code am Rechner angestöpselt ausführen zu lassen – was an den Doppelbelegungen der Pins liegt. Da landen nämlich auch die simulierten USB-Datenkanäle, was nicht immer schick ist, für das angehängte Sensor-Werk…

Man wird sich also daran gewöhnen, den Digispark per zusätzlicher Stromversorgung zu betanken, was recht komfortabel ist, denn man kann entweder USB-like 5V anlegen oder über den Wandler gehen, der 7-35V verträgt. Ich denke, die oberen Werte sollte man besser nicht wählen, sonst wird der Wandler auf Dauer einfach zu warm, aber nett zu wissen, dass man direkt am Boardnetz vom Auto damit operieren kann. Vielleicht bastle ich damit auch was fürs Moped? Mal sehen…

Die Teile des PeePoo 
---------------------

Dafür nehme man:

1.  einen Digispark
2.  ein TM1637 4-stelliges LED-Display (7-Segment)
3.  eine DS3231 RTC als Board mit zusätzlichem EEPROM-Chip (4kB) (ist bei mir ein China-übliches Dev-Board mit I2C-Schnittstelle und Aufdruck „Catalex“)
4.  2 Mini-Buttons
5.  ein Lochraster-Board mit Lötpunkten
6.  ein paar Streifen Sockelleisten in 2.54mm Standardmaß (2x6, 1x4, 1x3 Pins)
7.  3 Widerstände: 5K, 2K, 1K
8.  etwas Kabel, Lötkolben samt Zubehör
9.  einen 3D-Drucker wenns schick werden soll und man nicht besonders begabt in Holz- oder Metallbearbeitung ist
10. eine geeignete Stromversorgung
11. Geduld und Spucke (und genügend Ersatzkomponenten falls man sich mal „verbraten“ hat

Als erstes Projekt mit dem Digispark habe ich mir verkniffen, direkt etwas Batterie-optimiertes zu basteln. Das hebe ich mir für Folgeprojekte auf, so muss mal wieder in USB-Netzteil Dauerstrom geben.

<img src="7-Segment-Display TM1637 Catalex.jpg" width="50%" title="Das 7-Segment Display-Modul, 4-stellig, als Standard-China-Klon hier mit Beschriftung Catalex">
Ich habe mich auf dem Steckbrett bzw. weil gerade keines frei war, mit fliegender Verkabelung an die Einzelkomponenten herangetastet. Nachdem das Programmieren gut klappte, habe ich mit dem TM1637 begonnen, weil es immer gut ist, wenn man was ausgeben kann, under das kann der kleine von Haus aus ja nicht. Er hat kein Serial-Interface womit ich schön nebenbei debuggen kann…

Debugging ohne Serial
---------------------

Ich hatte noch von vor Urzeit manuell eine der Bibliotheken dafür heruntergeladen. Spannend an dem Board ist, dass es mit einem Two-Wire-Interface angesteuert wird, aber eben nicht über Standard I2C. Es bedarf also zweier Pins, einen für CLK und einen für DIO. womit mal wieder ein paar Schwierigkeiten begannen. Ich habe P3 und P2 ausgewählt, was spontan funktionierte. Da aber P2 später für die I2C-Schnittstelle zum RTC-Board benötigt wird, habe ich das wechseln müssen. Danach funktionierte gar nichts mehr. Nach einigem Experimentieren hat die Kombi P3/P4 funktioniert. Ich konnte später noch lesen, dass das vermutlich mit den weiteren Beschaltungen für die USB-Schnittstelle und die interne LED an P1 zu tun haben könnte. Da sind unerwartete Pull-Ups und ähnliches – ich habe auf jeden Fall nicht alles verstanden aber viel geflucht. Immerhin – an den Ports funktioniert das Display auch prima. Ich habe später noch so meine Probleme mit dem Doppelpunkt im Display gehabt - Spoiler: Es hängt am 8. Bit des 2. Zeichens…

Die „Echt-Zeit-Uhr“
-------------------

<img src="RTC DS3231SN ZS-042" width="50%" title="Das RTC-Modul ZS-042 auf Basis eines DS3231-SN plus einem eigenen EEPROM Chips">
Nachfolgend habe ich den dann freigewordenen Pins P0 und P2 das RTC-Board per I2C angeschlossen. Dort habe ich mich wegen der Programmgröße dazu entschieden, keine Library hierfür anzuwenden sondern direkt die Lese- und Setzfunktion zu schreiben und im Hauptprogramm abzulegen. Das war auch deswegen gut, weil für den Einsatz auf dem ATtiny die TinyWire-Lib anstelle der Wire-Lib empfohlen wird. Das ist aber nur ein begrenztes Drop-In-Replacement, also müsste eh was ändern, also kopier ich nur das wichtigste mit rüber.

Ein kurzes Auslesen und Zeit darstellen-Progrämmchen zeigt, dass der Parallelbetrieb Display und RTC nun klappt und die Kommunikation zur RTC ebenfalls funktioniert. Damit die Funktion geprüft wird und die RTC (hat schon eine Batterie bekommen) auch die korrekte Zeit liefert, setze ich einmal die Uhrzeit mit festen Werten über ein gut getimtes Einschalten des Digispark. Danach gleich wieder raus damit aus dem Code, es sollte ja nur einmal die Zeit gesetzt werden. (Das RTC-Modul hat übrigens einen Akku vorgesehen (LIR2032) – wenn man es mit einer Batterie betrieben möchte, muss ein Widerstand auf der Rückseite entfernt werden, damit das Modul nicht versucht die Batterie zu laden!)

EEPROM ist immer cool
---------------------

Ein Neustart zeigt nun tatsächlich die richtige Uhrzeit an – praktisch schon ein Erfolg. Angeblich soll die RTC sehr exakt laufen, ich bin gespannt. Jetzt ging es daran, das EEPROM auf dem RTC-Board auch anzusprechen. Ich möchte aus der Vergangenheit lernen und bei einem rein lokal laufenden System gegen Stromausfälle und Resets des Controllers gewappnet sein, also sollten die Zeiten der letzten Pee/Poo-Aktionen nachher im EEPROM gespeichert werden. Wenns hochkommt 8 Schreibvorgänge pro Tag – da brauche ich kein Wear-Leveling implementieren. Das hält…

Also kurz den Sketch um Kommunikationsfunktionen mit dem EEPROM ergänzt – was auch total easy ist, weil es auch per I2C angesteuert wird und das Internet bereits die Standard I2C-Adresse des EEPROMs auf dem Board verraten hat. Nun noch eine Testfunktion um einen Counter im EEPROM abzubilden und diesen anzeigen zu lassen. Zählt jede Sekunde um eins hoch, prima, Strom weg, Strom dran, zählt weiter, super, Strom ab, auch bei der RTC, Strom wieder dran, und zählt wieder weiter. Offensichtlich klappt die Kommunikation mit dem EEPROM und es ist wirklich eines :-)

2 Inputs, 1 Pin 
----------------

Das nächste Problem wartet auch schon: ich würde gerne zwei Knöpfe nutzen, einen fürs kleine Geschäft (Pee) und einen fürs Große (Poo). 2 Digitale Inputs – hm, ich hab noch P5 (Reset ist ja jetzt weg) und P1 (interne LED). P1 war mir allerdings weiterhin suspekt, daher hab ich überlegt, ob es auch anders geht. Natürlich – ist doch P5 auch als analoger Eingang nutzbar. Wie wäre es, wenn wir mit verschiedenen Widerständen gegen einen Pull-Down arbeiten um mit den beiden Knöpfen verschiedene Spannungen an P5 zu erzeugen. Im Internet (z.B. hier: <https://www.instructables.com/id/How-to-Multiple-Buttons-on-1-Analog-Pin-Arduino-Tu/>) erfahre ich, dass die Idee auch anderen so gut gefallen hat, dass sie die bereits umgesetzt haben.

<img src="Platine Unterseite ohne Baugruppen.jpg" width="50%" title="Die 'Board-Unterseite' - zwei der Widerstände gehen zu den Tastern.">
Also passend angeschlossen und einen Testsketch aufgesetzt, der „live“ den aktuellen analogen Wert von P5 anzeigt. Damit kann ich dann sauber testen ob meine Idee passt und welche Widerstände ich verwenden sollte. Nach ein wenig Experimentieren habe ich als Pull-Down einen 5K Widerstand verwendet und für die beiden Buttons einen 1K und 2K. Meine Werte hab ich ja ablesen können – sie warten gut auseinanderzuhalten, auch wenn beide Knöpfe zeitgleich gedrückt werden. Einzig gibt es immer wieder eine kleine Ungenauigkeit in der Auslesung, das verursacht leicht schwankende Werte. Tatsächlich kann ich aber für Messwerte x sagen:

-   x&lt;100 → keine Taste
-   100 &lt;= x &lt; 300 → Taste 1
-   300 &lt;= x &lt; 400 → Taste 2
-   x &gt; 400 → Taste 1 und 2

<img src="Analoge Messwerte mit linker Taste.jpg" width="50%" title="Hier wird die linke Taste gedrückt.">
Das kann natürlich bei anderen Beschaltungen, anderen Widerständen usw. anders sein, also selber ausmessen nicht vergessen. Mit dieser Info konnte ich dann Funktionen fürs Erkennen der Knöpfe samt Debouncing bauen.

Und jetzt „Platine frei“
------------------------

<img src="Platine Oberseite ohne Baugruppen.jpg" width="50%" title="Oben war ja schon die Unterseite zu sehen, hier nun die Oberseite - ohne Baugruppen.">
So, jetzt ist alles an Hardware möglich wie geplant, jetzt kommt Kampfverdrahtung auf der Lochrasterplatine. Ich hätte es gerne funktionierend, Oberseite einigermaßen klar, am Rand Freiflächen zur Positionierung und zum Festhalten des Boards am Gehäuse und natürlich eine gewisse Symmetrie vorne. Bei den Tasten ist das nicht ganz so wichtig, da kann man im Gehäusedesign noch was machen, aber das Display sollte mittig sitzen. Entgegen meiner üblichen Aufbauten sollte diesmal auch kein Teil der Devboards fest verlötet werden, sondern alles steckbar bleiben. Arretierungen müssen also im Gehäusedesign vorgesehen werden. Ich bin dann beim im Foto sichtbaren Design gelandet. Verdrahtung ist sicher optimierbar, ein eigenes PCB kann ich nicht, schöne Leitungsführung kann ich leider auch nicht. Aber funktionierend – das kann ich.

<img src="Platine mit Baugruppen Seitenansicht.jpg" width="50%" title="Hier ein Blick von der Seite mit allen gesteckten Baugruppen">
Alle Tests nach dem Löten und Stecken sind erfolgreich. Das beruhigt ungemein – weiß jeder der schon mal bei einem „fertigen“ Design Fehler festgestellt hat und dann alles umlöten und vor allem debuggen muss. Jetzt muss also noch die Software fertig werden und das Gehäuse designed und gedruckt werden.

Ohoh, muss das so? (DigiSpark gebrickt...)
------------------------------------------

Beim weiteren Entwickeln trat auf einmal ein seltsames Problem auf: Ich habe den Digispark ja immer im Wechsel in meinem gelöteten Board und am Rechner stecken. Mit dem Uhren-Testprogramm stellte ich auf einmal fest, dass die Uhrzeit sofort nach dem Einstecken aktualisiert wird. Da sollte es doch die 5 Sekunden-Pause geben? Mist, tut sie aber nicht mehr. Natürlich wird der Digispark nun am Rechner auch nicht mehr erkannt, weil er offensichtlich den Boorloader entweder überspringt oder gar nicht mehr kennt. Grrrr. Ich wollte mir das Thema Hochvoltprogrammierung (hier eine verständliche Anleitung was man tun muss: <https://www.instructables.com/id/Simple-and-cheap-Fuse-Doctor-for-Attiny/>) doch erst viel später antun. Mal sehen, ob ich was mit dem ArduinoISP lösen kann…

<img src="Anschluss DigiSpark.jpg" width="50%" title="Hier muss der kleine Kerl rein.">
Erstmal aber wird der nächste Digispark vorbereitet, sprich erstmal die Stiftleisten angelötet. Es war mir beim ersten mal nicht sofort aufgefallen, jetzt aber fühlt sich das Anlöten irgendwie seltsam an. Die Lötstellen sehen auch anders aus? Ahh, richtig, da das Digispark Board ja den USB-Stecker nachahmt, ist es dicker als die üblichen Boards, daher schaut die Stiftleiste auf der Lötseite viel weniger aus dem Board raus.

Erstmal ein Gehäuse
-------------------

<img src="Gehäuseoberteil leer.jpg" width="50%" title="Oberteil mit den Drück-Laschen und den 'Verlängerungen' auf die Taster">
Aus Frust baue ich erstmal das Gehäuse. Über TinkerCAD wird munter gewerkelt und mit der Schieblehre hantiert bis ein Oberteil existiert, welches eine geeignete Größe hat, eine passende Aussparung für das Display und Gehäuselaschen mit Druckpunktverlängerung zum Bedienen der Knöpfe über das Gehäuse. Mangels Beherschung von Klick-Verschluss-Designs bastle ich wieder was zum Zusammenschrauben. Das hält auf jeden Fall, nimmt aber leider immer einigen Raum ein. Diesmal sind es 9mm Kästen mit 2,8mm Röhre als Schraubenführung, damit später 3,5x30er Schrauben alles zusammenhalten. Das Board halte ich über die vorgesehene „Quiet-Zone“ an den Rändern, also dort wo kein Bauteil sitzen darf. Da bietet das Gehäuse nun einen Raum, der ein wenig (4mm) schmaler ist, als die Platine. Dort ist der Rand des Gehäuses um diese 2mm je Seite stärker, ich mache aber noch auf Platinenbreite eine 1,6mm tiefe und 40,4mm lange Aussparung je Seite rein, dann sitzt das Board sauber in dieser Aussparung fest und wird dann später vom Rückenteil des Gehäuses in diese Aussparung gedrückt bzw. dort gehalten.

<img src="Gehäuseunterteil innen.jpg" width="50%" title="Hier die Innenseite des Gehäuse-Unterteils - man erkennt die Schraubenösen...">
Für den Rand wo Ober- und Unterteil aufsetzen gönne ich mir eine Zierleiste von 1mm Überstand auf dem Oberteil, die ich dann auf dem Unterteil wegnehmen muss. Dadurch kann man dann nicht durch den Spalt schauen und auch Flüssigkeiten tropfen nicht direkt durch. Das fertige Teil habe ich freigegeben (<https://www.tinkercad.com/things/6aOmo4KOrsl>). Bis auf die Stromzuführung ist da schon alles drin, inklusive einer rückseitigen Aufhängung für Schrauben. Jetzt aber wieder zurück zu meinem Digispark.
<img src="Gehäuseunterteil von hinten.jpg" width="50%" title="... die man hier wie bei anderen Geräten erkennen kann"><img src="Gehäuseteile Kontaktnaht.jpg" width="50%" title="Eigentlich eine saubere Naht, siehe Lessons Learned...">


Andere Firmware?
----------------

Diesmal will ich mir anschauen, welche alternativen Firmware-Varianten es für den Bootloader gibt. Speziell eine, die meinen nun ungenutzten P1 verwenden könnte, fände ich prima – das muss doch machbar sein… Ich fange also an, gemäß der Anleitung (<https://frottier.wordpress.com/2017/11/12/kein-frust-mit-digispark-clones/>) den Digispark zu bearbeiten. Ich kann die neue Firmware nach dem Anpassen der Datei bootloaderconfig.h gemäß Anleitung compilieren und auch das Upgrade-Programm dazu bauen. Den neuen Digispark mit micronucleus –erase-only identifiziert – es ist noch Firmware 1.6 drauf. So, also Upgrade-Programm aufspielen und kurz warten. Ok, fertig – wieder mit erase-only nachsehen und tada – 2.4 ist nun drauf. Dann müsste jetzt ja alles… Ich lad einfach mal mein Test-Programm doch und – funktioniert sofort. Interessant – weil blöd. Ich kann das Biest nach dem Einstecken immer sofort beschreiben. Es ist also bei den 5 Sekunden geblieben, obwohl anders beschrieben. Grrrr, dann also Fehleranalyse.

Mist, keine Firmware, kein IO-Pin, oder? (Reset gerettet)
---------------------------------------------------------

Sehr seltsam, ich flashe definitiv die richtige Firmware. In der bootloaderconfig.h werden drei Makros für den Bootloader in Abhängigkeit zur gewählten Einstellung vorgenommen. Init und Exit sidn mit Funktionen belegt, die die Pins passend schalten und die StartCondition muss true liefern, was sie über die passende Pin-Abfrage auch sollte. Ein Test mit veränderter Wartedauer AUTO\_EXIT\_MS war erfolgreich, sprich – die Firmware wird korrekt verwendet. Alles weiterhin seltsam. Ich habe unabhängig davon aber auch nach anderen Möglichkeiten gesucht und gefunden, dass man sehr wohl auch mit einem Reset-P5 diesen noch als Eingang verwenden kann. (Siehe <http://www.technoblogy.com/show?LSE>) Das wird dadurch erreicht, dass die Spannung immer sicher über 2,2V bleibt, also immer ein HIGH-Signal anliegt. Ich habe ja die Knöpfe als Widerstandsleiter angeordnet. Ich muss also nur noch als generellen Spannungsteiler einen Widerstand direkt von P5 zu 5V ziehen. Der Schnelltest mit einem 1K Widerstand war erfolgreich. Schön ist es nicht, weil er so recht viel Strom zieht (Worst Case I=U/R, also 5V/1k Ohm = 5mA) aber es ist ja erstmal eine Bastellösung. Außerdem hab ich doch schon alles fertig gelötet \*heul\*…

<img src="Platine Unterseite ohne Baugruppen.jpg" width="50%" title="Der dritte Widerstand geht zum 5V-Pin.">
Ok, dieser Widerstand wird meine Knopfauslesewerte erheblich verändern, also nochmal das Testprogramm für die Analogwerte von P5 auf den Digispark geladen und neu gemessen… Die neuen Werte sind 514 im Leerlauf, 558 linker Knopf, 614 rechter Knopf, 644 beide Knöpfe. Wie schon vorher kommt es zu leichten „Zuckungen“ und die Werte liegen in einem Bereich von +-12 um diesen Wert, was für eine saubere Unterscheidung reicht. Ins Programm kommt eine reine Lesefunktion readButtons() samt Zuordnung, aber ich möchte sowohl ein Debouncing als auch eine Unterscheidung Links/Rechts/Beide und sowohl Drücken als auch Halten. Dafür gibt es eine separate Funktion checkButtons(), nach der in den drei globalen Variablen buttonStateXXX (XXX=Left, Right, Both) nun steht, ob ein Button gar nicht oder kurz gedrückt wurde oder ob er aktuell gehalten wird. Nach einer kleinen Debugging Session klappt nun das Button Handling.

Der Arduino-Sketch
------------------

Im eigentlichen Programm arbeiten wir nach einer kleinen State Machine um die Anzeige zu koordinieren. Einmal für den Modus in dem wir uns aktuell befinden sowie für die Anzeige-Zwischenschritte dafür. Wir müssen auf jeden Fall bereits einen Testmodus einbauen – denn wenn man keine andere Anzeige hat, ist es wichtig, Fehleranalyse im Gerät selbst zu betreiben. Dazu wende ich zwei Methoden an: Grundsätzlich gibt es eine Boot-Meldung, die zeitgleich der Diagnose dient. Zum einen – eine korrekte Erkennung mal vorausgesetzt – soll man den Testmodus erreichen, wenn die linke Taste während der Meldung gehalten wird. Was aber wenn die Tastenerkennung stockt? Dafür gibt es einen zweiten Trick. Da wir im EEPROM der RTC Werte „überwintern“ lassen können, also einen Reboot überleben lassen können, setzen wir während der Bootmeldung einfach ein Flag im EEPROM. Dieses Flag setzen wir am Ende der Bootmeldung einfach zurück. Der Trick besteht nun darin, dieses Flag zu Beginn der Bootmeldung – noch bevor wir es setzen – auszulesen. Steht unser Debug-Wert darin, dann wurde das Gerät während der Bootmeldung ausgeschaltet und danach wieder eingeschaltet. Das ist ein Weg, der auch ohne Tasten funktioniert. Ich habe die setup() Funktion gewählt um dort die Bootmeldung anzeigen zu lassen. In dieser Phase habe ich ohne Loop „volle Kontrolle“ über das System und weiß was bis hierher passiert sein sollte. Da wir im RTC EEPROM ab Adresse 0x60 „frei“ in der Nutzung sind, habe ich dieses Flag dort abgelegt.

Nach und nach werden alle States ergänzt inklusive der Komfortfunktion des Uhren-Stellens. Ich habe alles drin, keine Codefehler aber beim letzten übersetzen dann der Schock: „Der Sketch verwendet 6678 Bytes (111%)“ Au Backe, da war ich wohl zu fleißig. Mein C-Stil ist natürlich unter aller Kanone und zudem eher nicht auf Microcontroller ausgelegt. Mal sehen wo ich hier ohne funktionale Einschnitte Platz sparen kann.

Flash-Speicher sparen...
------------------------

Ok, wenn ich auf die Einstellung des Jahres verzichte, spare ich schon mal 300 Bytes ein. Das scheint ein guter Anfang, aber eigentlich mag ich das nicht einsparen… Naja, wichtig ist ja erstmal nur die Uhrzeit – also streiche ich auch die Datumseinstellung. Da sowieso keine automatische Zeitumstellung eingbaut wird (wer weiß wie lange es die noch gibt…) ist das vermutlich auch kein Beinbruch. Man ist von so einem ESP ganz schön verwöhnt… Bingo – ist der ganze Teil auskommentiert, bin ich bei 5712 Bytes oder 95%. Leben am Limit, aber was solls.

Der erste Test ist schon äußerst erfolgreich. Die Bedienung klappt wie am Schnürchen, alle Elemente funktionieren wie gewollt. Lediglich fällt mir auf, dass der Titel des Modus beim Rückwechsel zur Uhr irgendwie störend ist und weg könnte. Das ist schnell gemacht, weil ich nach dem Modus-Wechsel-Kommando den Submodus direkt auf 1 setzen kann. Es mehr Sorge macht mir ein zweiter Punkt, der praktisch den einzigen Bug darstellt. Mit dem Halten der zwei Tasten zum nächsten Element zu wechseln ist prima, aber beim Wechsel von Stunde einstellen auf Minute einstellen werden sofort die noch gehaltenen Buttons wieder erkannt und damit faktisch die Minuteneinstellung übersprungen. Sowas hatte ich geahnt und und an den meisten Stellen bereits vorbereitet – leider klappt das bei diesem Wechsel nicht wie gedacht. Ich werde wohl einfach hart einen weiteren Submodus einbauen, der wartet bis die Tastenkombi losgelassen wird, so wie bei den Texteinblendungen auch.

Mit diesen Änderungen bin ich bei 5750 Bytes. Ein Test ergibt aber neue Probleme. Nun wird am Ende der Zeiteinstellung nicht mehr auf Knopfdruck-Freiheit gewartet. Außerdem fällt mir ein, dass ich noch nicht an allen Stellen den Titel „Uhr“ überspringe. Also nochmal Finetuning… Bei der Gelegenheit stelle ich noch die Einstellung der Minuten beim Halten auf 4 pro Sekunde um, dann geht das Einstellen der Minuten schneller. So, nach diesem Tuning sind es 5770 Bytes. Da geht noch was :-) Ich liebäugle mit einer Einstellungsmöglichkeit für die Helligkeit.

… aber auch kein Byte verschenken
---------------------------------

Also los, das wird schon passen. Erstmal einen Schriftzug für die Einstellung bauen (SEG\_HELL) und eine globale Variable einfügen. Wir definieren auch Min- und Max-Werte, manche Werte ergeben gemäß Internet wohl keinen Sinn. Außerdem möchten wir natürlich, dass die Helligkeitseinstellung dauerhaft gespeichert wird, also sehen wir im EEPROM dafür eine weitere Adresse vor (BRIGHTNESS\_ADDRESS). Alles umgesetzt aber grrrr – 6082 Bytes. Noch ein wenig an der Halteerkennung der Tasten gefeilt, jetzt sind es 6002 Bytes (99%). Ich bin gespannt.

Ladies and Gentlemen, we have a product :-) Im Kopf geht ja die Featuriris mir mir durch, natürlich wäre eine WLAN-Anbindung mit Mitteilung über MQTT toll, dann könnte man auch „remote“ anzeigen, wann der Hund draußen war, z.B. im Schlafzimmer. Ich belasse es aber mal dabei auch wenn ich das Gehäuse hinten so gestaltet habe, dass auch ein größerer Chip (Wemos D1 Mini) noch reinpassen würde. Das Gerät soll erstmal zeigen, was es kann, daher geht es nun so an den Start. Für die letzte Aktion muss nur noch ein USB-Kabel gefunden und durch das Gehäuse geführt werden – diese Durchführung hatte ich ja im Design vergessen. Ein kleiner Metallbohrer wird’s schon richten und ein passendes Loch für das Kabel schaffen. Das Kabel bekommt hinter dem Loch im Gehäuse natürlich einen Knoten als Zugentlastung.

Fazit und Lessons Learned
-------------------------

So, nach dem Anlöten der weiblichen Dupont-Pfostenstecker kann das ganze nun verschraubt werden. Tatsächlich ist zum Schluss alles aus einem Guss. Ein paar Kleinigkeiten jedoch stören mich:

1.  Man sieht es nicht, aber das Loch für die Stromleitung hätte ich lieber gedruckt gehabt.
2.  Die geplante überlappende Kante wo sich beide Gehäuseseiten treffen ist nicht so dicht wie gedacht. Das liegt offensichtlich daran, dass das Unterteil oben und unten keinen „Innenteil“ mit im Design hat oder dieser beim Druck irgendwie abhanden kam.
3.  Dank der reinen Steckverbindungen und ohne extra Halterungen im Gehäuse ist das Uhrenmodul schlicht etwas beweglich und klappert wenn man das Gehäuse schüttelt.
4.  Das Display oben liegt nicht plan auf. Das ist wohl der besonderen Konstruktion (nur stecken) geschuldet. Hier hätte im Gehäusedesign die Innenkante schlicht bis auf das Boardr herunterreichen dürfen. Sowieso bin ich meinem üblichen Problem erlegen, dass im 3D-Modeller alles gigantisch groß aussieht, in der Realität dann aber doch vieles kleiner ist. Die Kante ist äußerst knappund bietet so nicht so viel Führung wie gewünscht.
5.  Mir ist erst am Ende aufgefallen, dass ich nur ein zu kurzes Kabel habe. Da würde ich gerne noch eine Lösung finden, die den Innenteil komplett aufbaut und an der Gehäusekante eine Steckverbindung (z.B. Hohlstecker oder Micro-USB) vorsieht.
6.  Das ganze Gehäuse ist etwas klobig, ich muss wohl im Design noch etwas im Punkt „Gefälligkeit fürs Auge“ arbeiten…
7.  Die Aufhänge-Nasen hinten sehen toll aus, sind aber wohl 2 mm zu klein um mit allen möglichen Schrauben klarzukommen. So sollte es keine Schraube mit Kopf &gt; 7 mm Durchmesser sein, was wohl 3,5 maximal wären.

Also – mal wieder viel an einem Projekt gelernt. In geringen Platzlagen ist der Digispark toll, wenngleich wie viele Microcontroller sehr sparsam mit IOs und Flash-Speicher. Die Passgenauigkeit des Gehäuses ist prima, bis auf den Überlappungsfehler (und mein schwaches Design) sind auch Elemente wie die Knöpfe prima benutzbar. Der Field-Test-Modus ist ebenfalls ein gutes Hilfsmittel. Und interaktiv eine Dokumentation wie diese anzufertigen ist ebenfalls hilfreich. Das fertige Produkt kann man übrigens hier in einem Video betrachten: <https://youtu.be/w1qvbLcAaJ8>
