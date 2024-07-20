Cache Simulation und Analyse- Translation Lookaside Buffer

Ein Translation Lookaside Buffer (TLB) ist ein Cache, in dem die jüngsten Übersetzungen von virtuellen in physische Speicheradressen gespeichert werden. Dadurch wird die für die Adressübersetzung erforderliche Zeit reduziert und die Effizienz des Speicherzugriffs verbessert. Ohne TLB kommt es bei jedem Speicherzugriff zu einer Latenzzeit beim Nachschlagen in der Seitentabelle, was die durchschnittliche Speicherzugriffszeit erhöht.

Moderne Prozessoren haben in der Regel zwei TLB-Ebenen:

L1 TLB: Klein und schnell, mit 32-128 Einträgen und einer Latenzzeit von 1 CPU-Zyklus.
L2 TLB: Größer und langsamer, mit 512-2048 Einträgen und einer Latenzzeit von etwa 10 CPU-Zyklen.
Ein TLB-Fehler erfordert die Übersetzung der Seitentabelle und den Zugriff auf den Hauptspeicher, was die Latenzzeit erheblich erhöht (100+ CPU-Zyklen).

Folgende TLB-Architekturen gibt es:

Vollständig assoziativ: Höhere Trefferquoten, aber komplexe Hardware.
Assoziativ gesetzt: Ein Gleichgewicht zwischen vollständig assoziativ und direkt zugeordnet.
Beispiele für Prozessoren:

Intel: L1 TLB ~64 Einträge, L2 TLB 512-1536 Einträge.
AMD: Ähnlich wie bei Intel.
ARM: L1 TLB 32-64 Einträge, L2 TLB variiert je nach Architektur.

Verkettete Listen haben ein zufälliges Zugriffsmuster, da die Knoten über den gesamten Speicher verteilt sind. Jeder Zugriff benötigt eine Zeigerdereferenzierung, was zu Zugriffen auf verschiedene Speicherseiten führt. Ohne eine TLB müsste das Betriebssystem bei jedem Zugriff virtuelle Adressen in physische Adressen übersetzen, was die Leistung verlangsamen würde. Eine TLB speichert die zuletzt verwendeten Adressübersetzungen, was schnelle Zugriffe ermöglicht. Bei einem TLB-Treffer erfolgt die Übersetzung sofort, bei einem TLB-Miss muss die Seitentabelle durchgesucht werden, was mehr Zeit in Anspruch nimmt.

Mit einem Input-File, das nicht die Summe einer Liste darstellt, ändert sich die Anzahl der Zyklen, wenn die TLB-Größe größer wird. Aber wenn wir die Summe einer verketteten Liste als Input-File eingeben bleibt die Anzahl der Zyklen gleich,obwohl die TLB-Größe geändert wurde.
Mit einem Input-File, das nicht die Summe einer Liste darstellt, ändert sich die Anzahl der Zyklen, wenn die TLB-Größe größer wird. Aber wenn wir die Summe einer verketteten Liste als Input-File eingeben bleibt die Anzahl der Zyklen gleich,obwohl die TLB-Größe geändert wurde. Case: 150-zeilige Input-File Parameters: cycles=1000 blocksize=64 Offset=32 TLB-size=32 tlb-latency=5 memory latency=10 Output: Cycles:1260 Hits:99 Misses:51

Altay Oezkan bearbeitete das Rahmenprogramm und die Themen Schlaltkreisanalyse und Optimierung für die Präsentation als. Paritosh Deshmukh arbeitete an makefile und half bei der Definition von Modulen in simulation.cpp sowie Problemstellung und Ausblick für die Präsentation. Shaurya Songara arbeitete an der Simulation und dem Forschungsteil der Readme und den Präsentationsthemen Korrektheit und Zusammenfassung. 



