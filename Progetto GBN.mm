<map version="freeplane 1.8.0">
<!--To view this file, download free mind mapping software Freeplane from http://freeplane.sourceforge.net -->
<node TEXT="Progetto GBN" FOLDED="false" ID="ID_383336953" CREATED="1627988627607" MODIFIED="1627999655854" STYLE="oval">
<font SIZE="18"/>
<hook NAME="MapStyle" background="#3c3f41">
    <properties edgeColorConfiguration="#808080ff,#00ddddff,#dddd00ff,#dd0000ff,#00dd00ff,#dd0000ff,#7cddddff,#dddd7cff,#dd7cddff,#7cdd7cff,#dd7c7cff,#7c7cddff" fit_to_viewport="false"/>

<map_styles>
<stylenode LOCALIZED_TEXT="styles.root_node" STYLE="oval" UNIFORM_SHAPE="true" VGAP_QUANTITY="24.0 pt">
<font SIZE="24"/>
<stylenode LOCALIZED_TEXT="styles.predefined" POSITION="right" STYLE="bubble">
<stylenode LOCALIZED_TEXT="default" ICON_SIZE="12.0 pt" COLOR="#cccccc" STYLE="fork">
<font NAME="SansSerif" SIZE="10" BOLD="false" ITALIC="false"/>
</stylenode>
<stylenode LOCALIZED_TEXT="defaultstyle.details"/>
<stylenode LOCALIZED_TEXT="defaultstyle.attributes">
<font SIZE="9"/>
</stylenode>
<stylenode LOCALIZED_TEXT="defaultstyle.note" COLOR="#cccccc" BACKGROUND_COLOR="#3c3f41" TEXT_ALIGN="LEFT"/>
<stylenode LOCALIZED_TEXT="defaultstyle.floating">
<edge STYLE="hide_edge"/>
<cloud COLOR="#f0f0f0" SHAPE="ROUND_RECT"/>
</stylenode>
</stylenode>
<stylenode LOCALIZED_TEXT="styles.user-defined" POSITION="right" STYLE="bubble">
<stylenode LOCALIZED_TEXT="styles.topic" COLOR="#18898b" STYLE="fork">
<font NAME="Liberation Sans" SIZE="10" BOLD="true"/>
</stylenode>
<stylenode LOCALIZED_TEXT="styles.subtopic" COLOR="#cc3300" STYLE="fork">
<font NAME="Liberation Sans" SIZE="10" BOLD="true"/>
</stylenode>
<stylenode LOCALIZED_TEXT="styles.subsubtopic" COLOR="#669900">
<font NAME="Liberation Sans" SIZE="10" BOLD="true"/>
</stylenode>
<stylenode LOCALIZED_TEXT="styles.important">
<icon BUILTIN="yes"/>
</stylenode>
</stylenode>
<stylenode LOCALIZED_TEXT="styles.AutomaticLayout" POSITION="right" STYLE="bubble">
<stylenode LOCALIZED_TEXT="AutomaticLayout.level.root" COLOR="#dddddd" STYLE="oval" SHAPE_HORIZONTAL_MARGIN="10.0 pt" SHAPE_VERTICAL_MARGIN="10.0 pt">
<font SIZE="18"/>
</stylenode>
<stylenode LOCALIZED_TEXT="AutomaticLayout.level,1" COLOR="#ff3300">
<font SIZE="16"/>
</stylenode>
<stylenode LOCALIZED_TEXT="AutomaticLayout.level,2" COLOR="#ffb439">
<font SIZE="14"/>
</stylenode>
<stylenode LOCALIZED_TEXT="AutomaticLayout.level,3" COLOR="#99ffff">
<font SIZE="12"/>
</stylenode>
<stylenode LOCALIZED_TEXT="AutomaticLayout.level,4" COLOR="#bbbbbb">
<font SIZE="10"/>
</stylenode>
<stylenode LOCALIZED_TEXT="AutomaticLayout.level,5"/>
<stylenode LOCALIZED_TEXT="AutomaticLayout.level,6"/>
<stylenode LOCALIZED_TEXT="AutomaticLayout.level,7"/>
<stylenode LOCALIZED_TEXT="AutomaticLayout.level,8"/>
<stylenode LOCALIZED_TEXT="AutomaticLayout.level,9"/>
<stylenode LOCALIZED_TEXT="AutomaticLayout.level,10"/>
<stylenode LOCALIZED_TEXT="AutomaticLayout.level,11"/>
</stylenode>
</stylenode>
</map_styles>
</hook>
<hook NAME="AutomaticEdgeColor" COUNTER="5" RULE="ON_BRANCH_CREATION"/>
<richcontent TYPE="DETAILS">

<html>
  <head>
    
  </head>
  <body>
    <p>
      client e server devono essere eseguiti nello spazio utente
    </p>
  </body>
</html>
</richcontent>
<node TEXT="Client" POSITION="right" ID="ID_1345677020" CREATED="1627988699435" MODIFIED="1627999655853" HGAP_QUANTITY="55.9999987483025 pt" VSHIFT_QUANTITY="-95.24999716132886 pt">
<edge COLOR="#00dddd"/>
<node TEXT="implementa comandi:" ID="ID_1613077386" CREATED="1627996315883" MODIFIED="1627998846541"><richcontent TYPE="DETAILS">

<html>
  <head>
    
  </head>
  <body>
    <p>
      questi invocati sono chiamati dal client e ritornano i loro valori al client
    </p>
  </body>
</html>
</richcontent>
<node TEXT="list()" ID="ID_254219289" CREATED="1627996330772" MODIFIED="1627998846539" HGAP_QUANTITY="85.2499978765846 pt" VSHIFT_QUANTITY="-122.2499963566662 pt"><richcontent TYPE="DETAILS">

<html>
  <head>
    
  </head>
  <body>
    <p>
      chiede al server di inviare la filelist
    </p>
  </body>
</html>
</richcontent>
</node>
<node TEXT="put(File file)" ID="ID_36680690" CREATED="1627996339460" MODIFIED="1627998734925" HGAP_QUANTITY="81.4999979883433 pt" VSHIFT_QUANTITY="-60.749998189508965 pt"><richcontent TYPE="DETAILS">

<html>
  <head>
    
  </head>
  <body>
    <p>
      invia al server il file argomento per poter essere caricato e attende la risposta
    </p>
  </body>
</html>
</richcontent>
</node>
<node TEXT="get(String fileName)" ID="ID_894154330" CREATED="1627996345736" MODIFIED="1627998799663" HGAP_QUANTITY="71.74999827891592 pt" VSHIFT_QUANTITY="89.24999734014281 pt"><richcontent TYPE="DETAILS">

<html>
  <head>
    
  </head>
  <body>
    <p>
      chiede al server di inviargli il file con il nome specificato e attende la risposta
    </p>
  </body>
</html>
</richcontent>
</node>
</node>
</node>
<node TEXT="Server" POSITION="left" ID="ID_1810678888" CREATED="1627988718499" MODIFIED="1627999645626" HGAP_QUANTITY="-634.7499806657438 pt" VSHIFT_QUANTITY="59.999998211860714 pt">
<edge COLOR="#dddd00"/>
<richcontent TYPE="DETAILS">

<html>
  <head>
    
  </head>
  <body>
    <ul>
      <li>
        Il server deve essere in ascolto su una porta di default (configurabile).
      </li>
      <li>
        concorrente
      </li>
    </ul>
  </body>
</html>
</richcontent>
<node TEXT="implementa comandi:" ID="ID_885084748" CREATED="1627988769579" MODIFIED="1627999572434" HGAP_QUANTITY="-489.99998497962997 pt" VSHIFT_QUANTITY="100.4999970048667 pt"><richcontent TYPE="DETAILS" HIDDEN="true">

<html>
  <head>
    
  </head>
  <body>
    <p>
      tutti questi comandi sono invocati dal client, dunque i ritorni sono riferiti al client.
    </p>
  </body>
</html>
</richcontent>
<node TEXT="list() -&gt; responseMsg" ID="ID_1930104107" CREATED="1627988816497" MODIFIED="1627989970278" HGAP_QUANTITY="-446.499986276031 pt" VSHIFT_QUANTITY="-92.9999972283841 pt"><richcontent TYPE="DETAILS">

<html>
  <head>
    
  </head>
  <body>
    <p>
      ritornare lista di tutti i files disponibili per la condivisione
    </p>
  </body>
</html>
</richcontent>
</node>
<node TEXT="get(String fileName) -&gt; responseMsg" ID="ID_1970482322" CREATED="1627988832649" MODIFIED="1627989333616" HGAP_QUANTITY="-492.24998491257475 pt" VSHIFT_QUANTITY="8.881784197001252E-16 pt"><richcontent TYPE="DETAILS">

<html>
  <head>
    
  </head>
  <body>
    <p>
      invia al client il messaggio di risposta con il file richiesto se presente, altrimenti con un errore.
    </p>
  </body>
</html>
</richcontent>
</node>
<node TEXT="put(File file) -&gt; responseMsg" ID="ID_1096970253" CREATED="1627988846013" MODIFIED="1627989968472" HGAP_QUANTITY="-460.74998585134784 pt" VSHIFT_QUANTITY="70.49999789893633 pt"><richcontent TYPE="DETAILS">

<html>
  <head>
    
  </head>
  <body>
    <p>
      salva tra i files disponibili per la condivisione il file argomento e ritorna un messaggio contenente l'esito dell'operazione
    </p>
  </body>
</html>
</richcontent>
</node>
</node>
</node>
<node TEXT="protocollo GBN" POSITION="right" ID="ID_700704023" CREATED="1627998929707" MODIFIED="1627999540246" HGAP_QUANTITY="-606.9999814927583 pt" VSHIFT_QUANTITY="-101.9999969601632 pt">
<edge COLOR="#dd0000"/>
<richcontent TYPE="DETAILS">

<html>
  <head>
    
  </head>
  <body>
    <ul>
      <li>
        Libreria in comune tra client e server che definisce le regole di comunicazione tra i due.
      </li>
      <li>
        Per comunicare client e server devono chiamare solo le funzioni di questa libreria
      </li>
    </ul>
  </body>
</html>
</richcontent>
<node TEXT="tre parametri" ID="ID_711802744" CREATED="1627999083029" MODIFIED="1628000981426" HGAP_QUANTITY="-384.9999881088737 pt" VSHIFT_QUANTITY="96.7499971166254 pt"><richcontent TYPE="DETAILS">

<html>
  <head>
    
  </head>
  <body>
    <p>
      questi parametri devono essere configurabili ed eguali per tutti i processi
    </p>
  </body>
</html>
</richcontent>
<node TEXT="timeout" ID="ID_1780479612" CREATED="1627999128609" MODIFIED="1628085384366" HGAP_QUANTITY="-603.2499816045171 pt" VSHIFT_QUANTITY="-100.49999700486671 pt"><richcontent TYPE="DETAILS">

<html>
  <head>
    
  </head>
  <body>
    <p>
      può essere o fisso o adattativo, in funzione dei ritardi osservati
    </p>
  </body>
</html>
</richcontent>
<node TEXT="timeout adattativo" ID="ID_623261887" CREATED="1627999726164" MODIFIED="1628000005749" HGAP_QUANTITY="-630.2499807998544 pt" VSHIFT_QUANTITY="-41.24999877065424 pt">
<node TEXT="\latex Timeout = EstimatedRTT + 4DevRTT" ID="ID_1933461354" CREATED="1627999803873" MODIFIED="1628085173833" HGAP_QUANTITY="-288.99999096989654 pt" VSHIFT_QUANTITY="69.74999792128807 pt">
<node TEXT="\latex EstimatedRTT = (1 - \alpha)EstimatedRTT + \alpha \cdot sampleRTT" ID="ID_1085281450" CREATED="1628000011249" MODIFIED="1628085676879" HGAP_QUANTITY="-538.7499835267668 pt" VSHIFT_QUANTITY="778.4999767988926 pt"><richcontent TYPE="DETAILS">

<html>
  <head>
    
  </head>
  <body>
    <ul>
      <li>
        sampleRTT è campionato su pacchetto trasmesso per la prima volta (non è stato ritrasmesso)
      </li>
      <li>
        EstimatedRTT inizializzato a zero
      </li>
      <li>
        alpha = 1/4
      </li>
    </ul>
  </body>
</html>
</richcontent>
</node>
<node TEXT="\latex DevRTT = (1 - \beta)DevRTT + \beta \cdot \abs {ampleRTT - estimatedRTT}" ID="ID_1694344365" CREATED="1628000263906" MODIFIED="1628085653561" HGAP_QUANTITY="-529.7499837949877 pt" VSHIFT_QUANTITY="-596.9999822080141 pt"><richcontent TYPE="DETAILS">

<html>
  <head>
    
  </head>
  <body>
    <ul>
      <li>
        DevRtt := 0
      </li>
      <li>
        beta = 1/4
      </li>
    </ul>
  </body>
</html>
</richcontent>
</node>
</node>
</node>
</node>
<node TEXT="winsize" ID="ID_996081609" CREATED="1627999135320" MODIFIED="1628085791684" HGAP_QUANTITY="-544.7499833479528 pt" VSHIFT_QUANTITY="-137.99999588727962 pt">
<node TEXT="\latex k = \frac{R \cdot RTT}{L} + 1" ID="ID_1661122594" CREATED="1628001002221" MODIFIED="1628085791682" HGAP_QUANTITY="-395.4999877959493 pt" VSHIFT_QUANTITY="53.249998413026375 pt"><richcontent TYPE="DETAILS">

<html>
  <head>
    
  </head>
  <body>
    <ul>
      <li>
        k = winsize
      </li>
      <li>
        R = rate di trasmissione
      </li>
      <li>
        L = grandezza messaggio
      </li>
      <li>
        se R e L sconosciuti, cronometrare tempo L/R e usare quello
      </li>
    </ul>
  </body>
</html>
</richcontent>
</node>
</node>
<node TEXT="\latex p" ID="ID_1628157194" CREATED="1627999139610" MODIFIED="1628085753880" HGAP_QUANTITY="-624.2499809786682 pt" VSHIFT_QUANTITY="-45.74999863654381 pt"><richcontent TYPE="DETAILS">

<html>
  <head>
    
  </head>
  <body>
    <p>
      probabilità che il messaggio sia scartato dal mittente anziché inviato
    </p>
  </body>
</html>
</richcontent>
</node>
</node>
</node>
</node>
</map>
