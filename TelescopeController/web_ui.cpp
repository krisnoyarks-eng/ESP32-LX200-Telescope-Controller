#include "web_ui.h"
#include "telescope.h"
#include "celestial.h"
#include "config.h"
#include <Preferences.h>

static ObserverSite* gSite = nullptr;
static Preferences prefs;

// Autostar II–style virtual handbox for Meade LX200 GPS
static const char INDEX_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8"/>
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no"/>
<title>Autostar II · LX200 GPS</title>
<style>
:root{--bg:#0a0a0c;--body:#1a1a1e;--key:#2a2a30;--key2:#222228;--lcdbg:#0c1a0c;--lcd:#7CFF7C;--lcdim:#3a6b3a;--acc:#c0c4cc;--red:#b33;--line:#333}
*{box-sizing:border-box;-webkit-tap-highlight-color:transparent}
body{margin:0;background:radial-gradient(ellipse at top,#1c1c22,#070708 60%);color:#ddd;font-family:Helvetica,Arial,sans-serif;min-height:100vh}
#setup{position:fixed;inset:0;background:#000c;z-index:50;display:flex;align-items:center;justify-content:center;padding:16px}
#setup.hidden{display:none}
.setup-card{background:#16161c;border:1px solid #3a3;border-radius:16px;padding:20px;max-width:420px;width:100%;box-shadow:0 20px 60px #000}
.setup-card h2{margin:0 0 8px;color:#7CFF7C;font-size:1.1rem}
.setup-card p{color:#999;font-size:.85rem;line-height:1.4;margin:0 0 14px}
h1{text-align:center;font-size:1rem;letter-spacing:.12em;color:#9aa;margin:14px 0 4px;font-weight:600}
.sub{text-align:center;color:#666;font-size:.75rem;margin:0 0 12px}
.layout{max-width:920px;margin:0 auto;padding:8px 12px 32px;display:grid;gap:16px}
@media(min-width:860px){.layout{grid-template-columns:380px 1fr}}
.handbox{background:linear-gradient(160deg,#25252b,#141418 50%,#1c1c20);border-radius:28px;padding:18px 16px 22px;border:1px solid #333;box-shadow:0 20px 50px #000a,inset 0 1px 0 #ffffff10;max-width:380px;margin:0 auto}
.brand{text-align:center;font-size:.65rem;letter-spacing:.2em;color:#777;margin-bottom:8px}
.lcd-bezel{background:#0a0a0a;border-radius:8px;padding:10px;border:2px solid #444;margin-bottom:12px;box-shadow:inset 0 0 20px #000}
.lcd{background:var(--lcdbg);border:1px solid #1a3a1a;border-radius:4px;padding:10px 12px;min-height:64px;font-family:"Courier New",ui-monospace,monospace;color:var(--lcd);font-size:1.05rem;line-height:1.35;text-shadow:0 0 6px #3f3;letter-spacing:.04em}
.lcd .dim{color:var(--lcdim);font-size:.72rem;margin-top:4px}
.row{display:flex;gap:8px;justify-content:center;margin:8px 0}
.row.spread{justify-content:space-between}
button{appearance:none;border:0;cursor:pointer;font-family:inherit;color:#e8e8ec;background:linear-gradient(180deg,#3a3a42,#222228);border-radius:10px;padding:12px 8px;font-size:.78rem;font-weight:700;letter-spacing:.04em;box-shadow:0 3px 0 #0d0d0f,0 4px 8px #0008;border:1px solid #4a4a52;user-select:none}
button:active{transform:translateY(2px);box-shadow:0 1px 0 #0d0d0f}
button.wide{flex:1;padding:14px 6px}
button.goto{background:linear-gradient(180deg,#4a3030,#2a1515);border-color:#633;color:#fcc}
button.dir{width:64px;height:48px;font-size:1.1rem;border-radius:12px}
button.num{width:31%;padding:14px 4px;background:linear-gradient(180deg,#323238,#1e1e22)}
button.num small{display:block;font-size:.62rem;font-weight:600;color:#9ab;letter-spacing:.06em;margin-top:2px}
button.scroll{width:48%;padding:10px;font-size:.9rem}
button.tiny{padding:8px 10px;font-size:.7rem;border-radius:8px}
.dpad{display:grid;grid-template-columns:64px 64px 64px;grid-template-rows:48px 48px 48px;gap:6px;justify-content:center;margin:10px 0}
.dpad .n{grid-column:2;grid-row:1}
.dpad .w{grid-column:1;grid-row:2}
.dpad .e{grid-column:3;grid-row:2}
.dpad .s{grid-column:2;grid-row:3}
.dpad .c{grid-column:2;grid-row:2;background:#151518;border-radius:50%;border:1px solid #333;display:flex;align-items:center;justify-content:center;font-size:.55rem;color:#555;pointer-events:none}
.keypad{display:flex;flex-wrap:wrap;gap:8px;justify-content:center}
.panel{background:#121216;border:1px solid #2a2a30;border-radius:16px;padding:14px}
.panel h2{margin:0 0 10px;font-size:.8rem;color:#888;letter-spacing:.1em;text-transform:uppercase}
.catalog{max-height:340px;overflow:auto;border:1px solid #2a2a30;border-radius:10px}
.catalog button{display:flex;width:100%;justify-content:space-between;border-radius:0;box-shadow:none;border:0;border-bottom:1px solid #222;background:#18181c;text-align:left;padding:10px 12px}
.catalog button:hover,.catalog button.on{background:#243018}
.catalog .mag{color:#8c8;font-family:monospace}
.filters{display:flex;flex-wrap:wrap;gap:6px;margin-bottom:8px}
.filters button{padding:7px 10px;font-size:.7rem}
.filters button.on{background:#2a4020;border-color:#4a6}
input{width:100%;background:#0c0c10;border:1px solid #333;color:#ddd;border-radius:8px;padding:8px;margin-bottom:8px}
.fieldrow{display:flex;flex-wrap:wrap;gap:8px}
.field{flex:1;min-width:90px}
.field label{display:block;font-size:.65rem;color:#777;margin-bottom:3px}
.hint{font-size:.72rem;color:#666;margin-top:8px;line-height:1.4}
#rawOut{font-family:monospace;font-size:.8rem;color:var(--lcd);background:#0c1a0c;border:1px solid #1a3a1a;border-radius:8px;padding:10px;min-height:44px;white-space:pre-wrap;word-break:break-all}
</style>
</head>
<body>
<div id="setup">
  <div class="setup-card">
    <h2>Setup · Start location</h2>
    <p>Like Autostar II first-time setup: enter where the telescope is (lat/lon) and local date/time before GO TO. Values are saved on the ESP32 and pushed to the LX200 GPS over RS232.</p>
    <div class="fieldrow">
      <div class="field"><label>Latitude ° (N +)</label><input id="su_lat" type="number" step="0.0001"/></div>
      <div class="field"><label>Longitude ° (E + / W −)</label><input id="su_lon" type="number" step="0.0001"/></div>
    </div>
    <div class="fieldrow">
      <div class="field"><label>UTC offset minutes</label><input id="su_tz" type="number"/></div>
    </div>
    <div class="fieldrow">
      <div class="field"><label>Year</label><input id="su_year" type="number"/></div>
      <div class="field"><label>Month</label><input id="su_month" type="number"/></div>
      <div class="field"><label>Day</label><input id="su_day" type="number"/></div>
    </div>
    <div class="fieldrow">
      <div class="field"><label>Hour</label><input id="su_hour" type="number"/></div>
      <div class="field"><label>Min</label><input id="su_minute" type="number"/></div>
      <div class="field"><label>Sec</label><input id="su_second" type="number"/></div>
    </div>
    <div class="row" style="justify-content:flex-start;margin-top:8px">
      <button class="tiny" onclick="fillSetupNow()">Use phone now</button>
      <button class="goto" style="flex:1" onclick="finishSetup()">Save site &amp; continue</button>
    </div>
  </div>
</div>
<h1>MEADE · AUTOSTAR II</h1>
<p class="sub">LX200 GPS · ESP32 Wi-Fi handbox · RS232</p>
<div class="layout">

<section class="handbox">
  <div class="brand">AUTOSTAR II · SMART FINDER</div>
  <div class="lcd-bezel">
    <div class="lcd" id="lcd">
      <div id="lcd1">RA --:--:--</div>
      <div id="lcd2">Dec --*--:--</div>
      <div class="dim" id="lcdDim">Ready · click a key</div>
    </div>
  </div>

  <div class="row spread">
    <button class="wide" onclick="pressEnter()">ENTER</button>
    <button class="wide" onclick="pressMode()">MODE</button>
    <button class="wide goto" onclick="pressGoto()">GO TO</button>
  </div>

  <div class="dpad">
    <button class="dir n" onpointerdown="mv('n')" onpointerup="hv('n')" onpointerleave="hv('n')" onpointercancel="hv('n')">▲</button>
    <button class="dir w" onpointerdown="mv('w')" onpointerup="hv('w')" onpointerleave="hv('w')" onpointercancel="hv('w')">◄</button>
    <div class="c">N/S/E/W</div>
    <button class="dir e" onpointerdown="mv('e')" onpointerup="hv('e')" onpointerleave="hv('e')" onpointercancel="hv('e')">►</button>
    <button class="dir s" onpointerdown="mv('s')" onpointerup="hv('s')" onpointerleave="hv('s')" onpointercancel="hv('s')">▼</button>
  </div>

  <div class="keypad">
    <button class="num" onclick="keySpeed()">1<small>SPEED</small></button>
    <button class="num" onclick="setFilter('cald')">2<small>CALD</small></button>
    <button class="num" onclick="setFilter('messier')">3<small>M</small></button>
    <button class="num" onclick="focusCmd()">4<small>FOCUS</small></button>
    <button class="num" onclick="setFilter('ss')">5<small>SS</small></button>
    <button class="num" onclick="setFilter('star')">6<small>STAR</small></button>
    <button class="num" onclick="reticleInfo()">7<small>RET</small></button>
    <button class="num" onclick="setFilter('ic')">8<small>IC</small></button>
    <button class="num" onclick="setFilter('ngc')">9<small>NGC</small></button>
    <button class="num" onclick="digit('0')">0<small>·</small></button>
  </div>

  <div class="row">
    <button class="scroll" onclick="scrollSel(-1)">▲ SCROLL</button>
    <button class="scroll" onclick="scrollSel(1)">SCROLL ▼</button>
  </div>

  <div class="row">
    <button class="tiny" onclick="cmd(':GR#')">Get RA</button>
    <button class="tiny" onclick="cmd(':GD#')">Get Dec</button>
    <button class="tiny" onclick="cmd(':GA#')">Alt</button>
    <button class="tiny" onclick="cmd(':GZ#')">Az</button>
    <button class="tiny" onclick="cmd(':GV#')">Ver</button>
    <button class="tiny" onclick="stopAll()" style="background:#402020">STOP</button>
  </div>
</section>

<section>
  <div class="panel">
    <h2>Object library · response</h2>
    <div id="rawOut">Ready. Commands print here instantly.</div>
    <div class="filters" style="margin-top:10px">
      <button class="on" data-f="all" onclick="setFilter('all',this)">ALL</button>
      <button data-f="ss" onclick="setFilter('ss',this)">SS</button>
      <button data-f="star" onclick="setFilter('star',this)">STAR</button>
      <button data-f="messier" onclick="setFilter('messier',this)">M</button>
      <button data-f="deep" onclick="setFilter('deep',this)">DEEP</button>
    </div>
    <input id="search" placeholder="Search / type M42…" oninput="renderCatalog()"/>
    <div class="catalog" id="catalog"></div>
    <p class="hint">Select object → <b>ENTER</b> loads target → <b>GO TO</b> slews (Autostar II). SPEED cycles Guide/Center/Find/Slew. Wire ESP32→MAX3232→LX200 GPS <b>RS232</b> jack (not HBX).</p>
  </div>

  <div class="panel" style="margin-top:12px">
    <h2>Site / time (Sun Moon planets)</h2>
    <div class="fieldrow">
      <div class="field"><label>Lat °</label><input id="lat" type="number" step="0.0001"/></div>
      <div class="field"><label>Lon °</label><input id="lon" type="number" step="0.0001"/></div>
      <div class="field"><label>UTC min</label><input id="tz" type="number"/></div>
    </div>
    <div class="fieldrow">
      <div class="field"><label>Y</label><input id="year" type="number"/></div>
      <div class="field"><label>M</label><input id="month" type="number"/></div>
      <div class="field"><label>D</label><input id="day" type="number"/></div>
      <div class="field"><label>h</label><input id="hour" type="number"/></div>
      <div class="field"><label>m</label><input id="minute" type="number"/></div>
      <div class="field"><label>s</label><input id="second" type="number"/></div>
    </div>
    <div class="row" style="justify-content:flex-start;margin-top:8px">
      <button class="tiny" onclick="useNow()">Use now</button>
      <button class="tiny" onclick="saveSite()">Save site</button>
      <button class="tiny" onclick="document.getElementById('setup').classList.remove('hidden')">Setup wizard</button>
      <button class="tiny" onclick="track(true)">Track ON</button>
      <button class="tiny" onclick="track(false)">Track OFF</button>
    </div>
  </div>
</section>
</div>
<script>
let catalog=[], filtered=[], selected=null, selIdx=0, filter='all', mode=0, status={}, setupDone=false;
const MODE_NAMES=['RA/Dec','Target','Alt/Az','Link'];
function el(id){return document.getElementById(id)}
function setLcd(a,b,dim){el('lcd1').textContent=a||'';el('lcd2').textContent=b||'';if(dim!=null)el('lcdDim').textContent=dim}
function showRaw(t){el('rawOut').textContent=t}
function fillSetupNow(){
  const d=new Date();
  el('su_year').value=d.getFullYear(); el('su_month').value=d.getMonth()+1; el('su_day').value=d.getDate();
  el('su_hour').value=d.getHours(); el('su_minute').value=d.getMinutes(); el('su_second').value=d.getSeconds();
  el('su_tz').value=-d.getTimezoneOffset();
  if(!el('su_lat').value) el('su_lat').value=40.015;
  if(!el('su_lon').value) el('su_lon').value=-105.2705;
}
async function finishSetup(){
  const body=new URLSearchParams({
    lat:el('su_lat').value,lon:el('su_lon').value,tz:el('su_tz').value,
    year:el('su_year').value,month:el('su_month').value,day:el('su_day').value,
    hour:el('su_hour').value,minute:el('su_minute').value,second:el('su_second').value,
    setup:1, push:1
  });
  const r=await fetch('/api/site',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});
  const t=await r.text();
  setupDone=true; el('setup').classList.add('hidden');
  setLcd('Site saved', el('su_lat').value+','+el('su_lon').value, 'Pushed to mount');
  showRaw(t); refreshStatus();
}
function kindOf(o){return ['star','planet','sun','moon','deep'][o.kind]||'obj'}
function passFilter(o){
  const k=kindOf(o), n=o.name.toUpperCase();
  if(filter==='all') return true;
  if(filter==='ss') return k==='planet'||k==='sun'||k==='moon';
  if(filter==='star') return k==='star';
  if(filter==='messier') return n.startsWith('M') && k==='deep';
  if(filter==='cald'||filter==='ic'||filter==='ngc') return k==='deep';
  if(filter==='deep') return k==='deep';
  return true;
}
function setFilter(f,btn){
  filter=f;
  document.querySelectorAll('.filters button').forEach(b=>b.classList.toggle('on', b.dataset.f===f || (f==='messier'&&b.dataset.f==='messier') || (f==='ss'&&b.dataset.f==='ss') || (f==='star'&&b.dataset.f==='star') || (f==='deep'&&b.dataset.f==='deep') || (f==='all'&&b.dataset.f==='all')));
  if(btn){document.querySelectorAll('.filters button').forEach(b=>b.classList.remove('on'));btn.classList.add('on')}
  const labels={ss:'Solar System',star:'Stars',messier:'Messier',cald:'Caldwell/Deep',ic:'IC/Deep',ngc:'NGC/Deep',all:'All objects'};
  setLcd('Object Library', labels[f]||f, 'Scroll to browse · ENTER selects');
  showRaw('Filter → '+(labels[f]||f));
  renderCatalog();
}
function renderCatalog(){
  const q=(el('search').value||'').toLowerCase();
  filtered=catalog.filter(passFilter).filter(o=>o.name.toLowerCase().includes(q));
  const box=el('catalog'); box.innerHTML='';
  filtered.forEach((o,i)=>{
    const b=document.createElement('button');
    b.className=(selected&&selected.name===o.name)?'on':'';
    b.innerHTML=`<span>${o.name} <small style="color:#666">${kindOf(o)}</small></span><span class="mag">${o.mag.toFixed(1)}</span>`;
    b.onclick=()=>{selIdx=i;selected=o;renderCatalog();setLcd('Select:', o.name, 'ENTER confirms · GO TO slews');showRaw('Selected '+o.name)};
    box.appendChild(b);
  });
  if(selIdx>=filtered.length)selIdx=Math.max(0,filtered.length-1);
}
function scrollSel(d){
  if(!filtered.length)return;
  selIdx=(selIdx+d+filtered.length)%filtered.length;
  selected=filtered[selIdx];
  renderCatalog();
  const node=el('catalog').children[selIdx]; if(node)node.scrollIntoView({block:'nearest'});
  setLcd('Select:', selected.name, 'ENTER · GO TO');
}
async function cmd(c){
  setLcd('Sending...', c, 'Wi-Fi → ESP32 → RS232');
  showRaw('Sending '+c+' ...');
  try{
    const r=await fetch('/execute?cmd='+encodeURIComponent(c));
    const t=await r.text();
    showRaw(c+' → '+t);
    setLcd(c, t.replace(/#/g,''), 'LX200 GPS response');
    refreshStatus();
  }catch(e){showRaw('Error '+e);setLcd('ERROR', String(e),'')}
}
async function mv(d){fetch('/api/move?dir='+d,{method:'POST'});setLcd('MOVE', d.toUpperCase(), 'Hold to slew')}
async function hv(d){fetch('/api/halt?dir='+d,{method:'POST'})}
async function stopAll(){await fetch('/api/stop',{method:'POST'});setLcd('STOP','Motors halted','');showRaw('STOP :Q#');refreshStatus()}
async function track(on){await fetch('/api/track?on='+(on?1:0),{method:'POST'});setLcd(on?'Tracking ON':'Tracking OFF','','');refreshStatus()}
async function keySpeed(){const r=await fetch('/api/speed',{method:'POST'});const t=await r.text();setLcd('SPEED',t,'Autostar rate');showRaw('SPEED → '+t);refreshStatus()}
async function focusCmd(){await cmd(':FQ#');setLcd('FOCUS','Halt focus','4/FOCUS')}
function reticleInfo(){setLcd('RETICLE','Use base RETICLE jack','Not serial-controlled');showRaw('RET → physical RETICLE port on LX200 GPS base')}
function digit(d){el('search').value+=d;renderCatalog()}
async function pressEnter(){
  if(!selected){setLcd('ENTER','No object','Pick from library');return}
  setLcd('Target loaded', selected.name, 'Press GO TO');
  showRaw('ENTER → target '+selected.name);
  await fetch('/api/select?name='+encodeURIComponent(selected.name),{method:'POST'});
  refreshStatus();
}
async function pressGoto(){
  if(!selected){setLcd('GO TO','No target','ENTER an object first');return}
  setLcd('GO TO', selected.name, 'Slewing via :MS#');
  showRaw('GO TO '+selected.name+' ...');
  const r=await fetch('/api/goto?name='+encodeURIComponent(selected.name)+'&track=1',{method:'POST'});
  const t=await r.text();
  showRaw(t); setLcd('GO TO', t.substring(0,40), 'Tracking armed');
  refreshStatus();
}
function pressMode(){
  mode=(mode+1)%4;
  if(mode===0) setLcd('RA '+ (status.ra||'--'), 'Dec'+(status.dec||'--'), MODE_NAMES[mode]);
  else if(mode===1) setLcd(status.target||'No target', status.rate||'', MODE_NAMES[mode]);
  else if(mode===2) setLcd('Alt '+(status.alt||'--'), 'Az '+(status.az||'--'), MODE_NAMES[mode]);
  else setLcd(status.connected?'LX200 GPS OK':'DEMO MODE', status.version||'', MODE_NAMES[mode]);
  showRaw('MODE → '+MODE_NAMES[mode]);
}
async function refreshStatus(){
  try{
    const r=await fetch('/api/status'); status=await r.json();
    setupDone=!!status.setupDone;
    if(!setupDone){
      el('setup').classList.remove('hidden'); fillSetupNow();
      if(status.site){
        el('su_lat').value=status.site.lat; el('su_lon').value=status.site.lon; el('su_tz').value=status.site.tz;
      }
    } else { el('setup').classList.add('hidden'); }
    if(mode===0 && setupDone) setLcd('RA '+status.ra, 'Dec'+status.dec, (status.connected?'MOUNT':'DEMO')+' · '+status.rate);
    else if(document.activeElement && document.activeElement.tagName==='INPUT'){/* keep */}
    else if(mode===1) setLcd(status.target||'-', status.lcd2||status.rate||'', 'Target');
    if(status.site){
      const x=status.site;
      ['lat','lon','tz','year','month','day','hour','minute','second'].forEach(k=>{
        const e=el(k); if(e && document.activeElement!==e) e.value=x[k];
      });
    }
  }catch(e){}
}
function useNow(){
  const d=new Date();
  el('year').value=d.getFullYear(); el('month').value=d.getMonth()+1; el('day').value=d.getDate();
  el('hour').value=d.getHours(); el('minute').value=d.getMinutes(); el('second').value=d.getSeconds();
  el('tz').value=-d.getTimezoneOffset();
}
async function saveSite(){
  const body=new URLSearchParams({
    lat:el('lat').value,lon:el('lon').value,tz:el('tz').value,
    year:el('year').value,month:el('month').value,day:el('day').value,
    hour:el('hour').value,minute:el('minute').value,second:el('second').value,
    setup:1, push:1
  });
  const r=await fetch('/api/site',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});
  showRaw(await r.text()); refreshStatus();
}
async function loadCatalog(){const r=await fetch('/api/catalog');catalog=await r.json();renderCatalog()}
fillSetupNow(); loadCatalog(); refreshStatus(); setInterval(refreshStatus,1600);
</script>
</body>
</html>
)HTML";

static String jsonEscape(const String& s) {
  String o;
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '"' || c == '\\') { o += '\\'; o += c; }
    else if (c == '\n') o += "\\n";
    else o += c;
  }
  return o;
}

static bool gSetupDone = false;

static void loadSiteFromPrefs(ObserverSite& site) {
  prefs.begin("scope", true);
  site.latDeg = prefs.getFloat("lat", DEFAULT_LAT_DEG);
  site.lonDeg = prefs.getFloat("lon", DEFAULT_LON_DEG);
  site.tzMinutes = prefs.getInt("tz", DEFAULT_TZ_MIN);
  site.year = prefs.getInt("year", 2026);
  site.month = prefs.getInt("month", 8);
  site.day = prefs.getInt("day", 6);
  site.hour = prefs.getInt("hour", 21);
  site.minute = prefs.getInt("minute", 0);
  site.second = prefs.getInt("second", 0);
  gSetupDone = prefs.getBool("setupOk", false);
  prefs.end();
}

static void saveSiteToPrefs(const ObserverSite& site) {
  prefs.begin("scope", false);
  prefs.putFloat("lat", site.latDeg);
  prefs.putFloat("lon", site.lonDeg);
  prefs.putInt("tz", site.tzMinutes);
  prefs.putInt("year", site.year);
  prefs.putInt("month", site.month);
  prefs.putInt("day", site.day);
  prefs.putInt("hour", site.hour);
  prefs.putInt("minute", site.minute);
  prefs.putInt("second", site.second);
  prefs.putBool("setupOk", gSetupDone);
  prefs.end();
}

void webSetup(WebServer& server, ObserverSite& site) {
  gSite = &site;
  loadSiteFromPrefs(site);

  server.on("/", HTTP_GET, [&server]() {
    server.send_P(200, "text/html", INDEX_HTML);
  });

  server.on("/execute", HTTP_GET, [&server]() {
    if (!server.hasArg("cmd")) {
      server.send(400, "text/plain", "Missing command argument");
      return;
    }
    String result = telescope.sendRaw(server.arg("cmd"));
    telescope.refreshPosition(true);
    server.send(200, "text/plain", result);
  });

  server.on("/api/status", HTTP_GET, [&server]() {
    telescope.refreshPosition(false);
    ScopeStatus& st = telescope.status();
    int rate = st.slewRate;
    if (rate < 0 || rate > 3) rate = 3;
    String j = "{";
    j += "\"ra\":\"" + jsonEscape(st.ra) + "\",";
    j += "\"dec\":\"" + jsonEscape(st.dec) + "\",";
    j += "\"alt\":\"" + jsonEscape(st.alt) + "\",";
    j += "\"az\":\"" + jsonEscape(st.az) + "\",";
    j += "\"version\":\"" + jsonEscape(st.version) + "\",";
    j += "\"connected\":" + String(st.connected ? "true" : "false") + ",";
    j += "\"tracking\":" + String(st.tracking ? "true" : "false") + ",";
    j += "\"slewing\":" + String(st.slewing ? "true" : "false") + ",";
    j += "\"rate\":\"" + String(SLEW_RATE_NAMES[rate]) + "\",";
    j += "\"target\":\"" + jsonEscape(st.targetName) + "\",";
    j += "\"lcd1\":\"" + jsonEscape(st.lcd1) + "\",";
    j += "\"lcd2\":\"" + jsonEscape(st.lcd2) + "\",";
    j += "\"setupDone\":" + String(gSetupDone ? "true" : "false") + ",";
    j += "\"lastCmd\":\"" + jsonEscape(st.lastCmd) + "\",";
    j += "\"lastReply\":\"" + jsonEscape(st.lastReply) + "\",";
    j += "\"site\":{";
    j += "\"lat\":" + String(gSite->latDeg, 5) + ",";
    j += "\"lon\":" + String(gSite->lonDeg, 5) + ",";
    j += "\"tz\":" + String(gSite->tzMinutes) + ",";
    j += "\"year\":" + String(gSite->year) + ",";
    j += "\"month\":" + String(gSite->month) + ",";
    j += "\"day\":" + String(gSite->day) + ",";
    j += "\"hour\":" + String(gSite->hour) + ",";
    j += "\"minute\":" + String(gSite->minute) + ",";
    j += "\"second\":" + String(gSite->second);
    j += "}}";
    server.send(200, "application/json", j);
  });

  server.on("/api/catalog", HTTP_GET, [&server]() {
    String j = "[";
    size_t n = Celestial::catalogSize();
    for (size_t i = 0; i < n; i++) {
      const CelestialObject& o = Celestial::catalogAt(i);
      if (i) j += ",";
      j += "{\"name\":\"" + jsonEscape(String(o.name)) + "\",";
      j += "\"kind\":" + String((int)o.kind) + ",";
      j += "\"raHours\":" + String(o.raHours, 4) + ",";
      j += "\"decDeg\":" + String(o.decDeg, 4) + ",";
      j += "\"mag\":" + String(o.mag, 2) + "}";
    }
    j += "]";
    server.send(200, "application/json", j);
  });

  server.on("/api/select", HTTP_POST, [&server]() {
    if (!server.hasArg("name")) {
      server.send(400, "text/plain", "name required");
      return;
    }
    telescope.status().targetName = server.arg("name");
    telescope.updateLcd("Target loaded", server.arg("name"));
    server.send(200, "text/plain", "selected " + server.arg("name"));
  });

  server.on("/api/compute", HTTP_GET, [&server]() {
    if (!server.hasArg("name") || !gSite) {
      server.send(400, "application/json", "{\"ok\":false}");
      return;
    }
    int idx = Celestial::findByName(server.arg("name").c_str());
    if (idx < 0) {
      server.send(404, "application/json", "{\"ok\":false}");
      return;
    }
    const CelestialObject& o = Celestial::catalogAt((size_t)idx);
    SkyCoords c = Celestial::compute(o, *gSite);
    String j = "{\"ok\":true,\"name\":\"" + jsonEscape(String(o.name)) + "\",";
    j += "\"ra\":" + String(c.raHours, 6) + ",\"dec\":" + String(c.decDeg, 6) + ",";
    j += "\"raStr\":\"" + Telescope::formatRA(c.raHours) + "\",";
    j += "\"decStr\":\"" + Telescope::formatDec(c.decDeg) + "\"}";
    server.send(200, "application/json", j);
  });

  server.on("/api/goto", HTTP_POST, [&server]() {
    if (!server.hasArg("name") || !gSite) {
      server.send(400, "text/plain", "name required");
      return;
    }
    int idx = Celestial::findByName(server.arg("name").c_str());
    if (idx < 0) {
      server.send(404, "text/plain", "Object not found");
      return;
    }
    const CelestialObject& o = Celestial::catalogAt((size_t)idx);
    SkyCoords c = Celestial::compute(o, *gSite);
    bool okRa = telescope.setRA(c.raHours);
    bool okDec = telescope.setDec(c.decDeg);
    telescope.status().targetName = String(o.name);
    int slew = telescope.slewToTarget();
    bool doTrack = !server.hasArg("track") || server.arg("track") == "1";
    if (doTrack) telescope.setTracking(true);

    String msg = String("GO TO ") + o.name + " " + Telescope::formatRA(c.raHours) +
                 " " + Telescope::formatDec(c.decDeg) + " set=" +
                 String(okRa && okDec ? "OK" : "CHK") + " slew=";
    if (slew == 0) msg += "0(ok)";
    else if (slew == 1) msg += "1(horizon)";
    else if (slew == 2) msg += "2(align)";
    else msg += "?";
    server.send(200, "text/plain", msg);
  });

  server.on("/api/stop", HTTP_POST, [&server]() {
    telescope.stopAll();
    server.send(200, "text/plain", "Stopped");
  });

  server.on("/api/move", HTTP_POST, [&server]() {
    if (!server.hasArg("dir")) { server.send(400, "text/plain", "dir"); return; }
    char d = server.arg("dir").charAt(0);
    if (d >= 'A' && d <= 'Z') d = d - 'A' + 'a';
    telescope.move(d);
    server.send(200, "text/plain", "move");
  });

  server.on("/api/halt", HTTP_POST, [&server]() {
    if (!server.hasArg("dir")) { server.send(400, "text/plain", "dir"); return; }
    char d = server.arg("dir").charAt(0);
    if (d >= 'A' && d <= 'Z') d = d - 'A' + 'a';
    telescope.haltMove(d);
    server.send(200, "text/plain", "halt");
  });

  server.on("/api/track", HTTP_POST, [&server]() {
    bool on = server.hasArg("on") && server.arg("on") == "1";
    telescope.setTracking(on);
    server.send(200, "text/plain", on ? "tracking on" : "tracking off");
  });

  server.on("/api/speed", HTTP_POST, [&server]() {
    int r = telescope.cycleSlewRate();
    server.send(200, "text/plain", SLEW_RATE_NAMES[r]);
  });

  server.on("/api/site", HTTP_POST, [&server]() {
    if (!gSite) { server.send(500, "text/plain", "no site"); return; }
    if (server.hasArg("lat")) gSite->latDeg = server.arg("lat").toFloat();
    if (server.hasArg("lon")) gSite->lonDeg = server.arg("lon").toFloat();
    if (server.hasArg("tz")) gSite->tzMinutes = server.arg("tz").toInt();
    if (server.hasArg("year")) gSite->year = server.arg("year").toInt();
    if (server.hasArg("month")) gSite->month = server.arg("month").toInt();
    if (server.hasArg("day")) gSite->day = server.arg("day").toInt();
    if (server.hasArg("hour")) gSite->hour = server.arg("hour").toInt();
    if (server.hasArg("minute")) gSite->minute = server.arg("minute").toInt();
    if (server.hasArg("second")) gSite->second = server.arg("second").toInt();
    if (server.hasArg("setup") && server.arg("setup") == "1") gSetupDone = true;
    saveSiteToPrefs(*gSite);

    String msg = "Site saved";
    if (server.hasArg("push") && server.arg("push") == "1") {
      telescope.pushSiteToMount(
        gSite->latDeg, gSite->lonDeg,
        gSite->year, gSite->month, gSite->day,
        gSite->hour, gSite->minute, gSite->second,
        gSite->tzMinutes
      );
      msg += " + pushed to LX200 GPS (:St :Sg :SL :SC :SG)";
    }
    telescope.updateLcd("Site ready", String(gSite->latDeg, 2) + "," + String(gSite->lonDeg, 2));
    server.send(200, "text/plain", msg);
  });
}

void webLoopExtras() {}
