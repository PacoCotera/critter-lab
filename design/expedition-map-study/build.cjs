// Offline paper/state study only. This is not an executable expedition game.
// Existing bundled Node + Sharp; no browser, dependency install or native build.
const fs = require('fs');
const path = require('path');
const crypto = require('crypto');
const sharp = require('sharp');
const root = path.resolve(__dirname, '../..');
const out = path.join(__dirname, 'exports');
const source = path.join(__dirname, 'source');
fs.mkdirSync(out, {recursive:true});
fs.mkdirSync(source, {recursive:true});
const P = {graphite:'#1e282f',field:'#202b32',shadow:'#0b1821',blue:'#2389c6',cyan:'#67cef5',ink:'#e3edef',muted:'#a5b6bd',warm:'#f1cd79',saved:'#a3cda8'};
const assets = ['data-compact','energy-compact','essence-compact','sample-neutral','data-primary','energy-primary','essence-primary'];
const hash = buffer => crypto.createHash('sha256').update(buffer).digest('hex');
const esc = value => String(value).replaceAll('&','&amp;').replaceAll('<','&lt;').replaceAll('>','&gt;');
const manifest = {status:'Provisional static expedition proposal; no runtime, balance or physical-display claim',palette:P,assets:[],references:[],screens:[],maps:[],textBounds:[],proofs:[]};
const images = new Map();
const rect = (x,y,w,h,fill) => `<rect x="${x}" y="${y}" width="${w}" height="${h}" fill="${fill}"/>`;
const line = (x1,y1,x2,y2,c,w=1) => `<path d="M${x1} ${y1}H${x2}V${y2}" fill="none" stroke="${c}" stroke-width="${w}"/>`;
function shoulder(x,y,w,h,inset,fill) {
  return `<path d="M${x+inset} ${y}H${x+w-inset}V${y+6}H${x+w}V${y+h-6}H${x+w-inset}V${y+h}H${x+inset}V${y+h-6}H${x}V${y+6}H${x+inset}Z" fill="${fill}"/>`;
}
function panel(x,y,w,h) {
  return shoulder(x+3,y+4,w,h,12,'#09141b')+shoulder(x,y,w,h,12,'#08151e')+shoulder(x+1,y+1,w-2,h-2,11,P.blue)+shoulder(x+3,y+3,w-6,h-6,9,'#41515a')+shoulder(x+5,y+5,w-10,h-10,7,P.field)+rect(x+14,y+1,Math.min(w-30,82),1,P.cyan)+rect(x+1,y+10,1,30,P.cyan);
}
function focus(x,y,w,h) {
  return shoulder(x-4,y-4,w+8,h+8,8,'#30332d')+shoulder(x-2,y-2,w+4,h+4,7,'#544830')+shoulder(x,y,w,h,6,'#0a141c')+shoulder(x+2,y+2,w-4,h-4,4,'#b9914d')+shoulder(x+3,y+3,w-6,h-6,3,P.field)+rect(x+12,y+2,w-24,1,P.warm);
}
function rng(seed) {let value=seed>>>0;return ()=> {value^=value<<13;value^=value>>>17;value^=value<<5;return (value>>>0)/4294967296;};}
function tree(x,y,variant=0) {
  const c=variant?['#1b3934','#32644d','#719566']:['#203f39','#45734f','#90a169'];
  return rect(x+7,y+18,4,7,'#564637')+rect(x+2,y+13,15,7,c[0])+rect(x+4,y+7,11,9,c[1])+rect(x+7,y+2,6,8,c[2])+rect(x+2,y+13,5,3,c[1])+rect(x+12,y+16,4,3,'#2a533f');
}
function rock(x,y) {return rect(x+3,y+5,15,11,'#17272c')+rect(x+3,y+2,11,11,'#536d68')+rect(x+5,y,7,3,'#91a49a')+rect(x+3,y+8,4,3,'#354a48')+rect(x+12,y+4,5,5,'#405650');}
function marker(x,y,type,status) {
  let body='';
  if(type==='camp')body=rect(x-9,y-5,18,12,'#172a30')+`<path d="M${x-8} ${y+5}L${x} ${y-9}L${x+8} ${y+5}Z" fill="#b4aa83"/>`+rect(x-2,y,4,7,'#263e3c');
  else if(type==='brook')body=rect(x-8,y-6,16,13,'#325f55')+rect(x-8,y-3,16,4,'#71cbd0')+rect(x-4,y-8,5,4,'#9eaf74')+rect(x+3,y+4,6,3,'#92b4a8');
  else if(type==='relay')body=rect(x-6,y-8,12,16,'#315d65')+rect(x-3,y-6,6,9,'#88b9b8')+rect(x-9,y+6,18,4,'#263a3b')+rect(x-1,y-12,2,5,'#b6c7be');
  else if(type==='ridge')body=rock(x-10,y-8)+rect(x-8,y+8,15,2,'#9aa584');
  else body=rect(x-8,y-5,16,11,'#283b3b')+rect(x-8,y-5,16,3,'#7f9a89')+rect(x-3,y-5,6,11,'#9ab4a2');
  let surround='';
  if(status==='inspected')surround=`<rect x="${x-12}" y="${y-13}" width="24" height="26" fill="none" stroke="${P.cyan}" stroke-width="1"/>`;
  if(status==='active')surround=rect(x-12,y+13,24,2,P.warm)+rect(x-12,y+12,2,4,P.warm)+rect(x+10,y+12,2,4,P.warm);
  if(status==='unknown')body=rect(x-7,y-7,14,14,'#3d5050')+rect(x-2,y-4,5,2,'#bcc9b5')+rect(x+1,y-2,2,4,'#bcc9b5')+rect(x-1,y+3,2,2,'#bcc9b5');
  if(status==='resolved')body+=rect(x+7,y+5,3,3,P.saved)+rect(x+10,y+2,3,3,P.saved);
  return surround+body;
}
function mapGeometry(seed, layout, lead=false) {
  const random=rng(seed), W=400,H=220,T=20;
  const walk=new Set();
  const routes=[...layout.routes,...(lead?[layout.leadRoute]:[])].map(route=>route.map(id=>layout.nodes.find(n=>n.id===id)));
  for(const route of routes) for(let i=1;i<route.length;i++) {
    const a=route[i-1],b=route[i];let x=a.x,y=a.y;
    walk.add(`${x},${y}`);
    if(seed===7183&&b.id==='brook'){while(y!==b.y){y+=Math.sign(b.y-y);walk.add(`${x},${y}`);}while(x!==b.x){x+=Math.sign(b.x-x);walk.add(`${x},${y}`);}}
    else{while(x!==b.x){x+=Math.sign(b.x-x);walk.add(`${x},${y}`);}while(y!==b.y){y+=Math.sign(b.y-y);walk.add(`${x},${y}`);}}
  }
  let body=rect(0,0,W,H,'#32483e');
  for(let y=0;y<11;y++)for(let x=0;x<20;x++){
    const r=random();body+=rect(x*T,y*T,T,T,r<.3?'#344c40':r<.6?'#38503f':'#3c5342');
    for(let k=0;k<3;k++){const dx=Math.floor(random()*8)*2,dy=Math.floor(random()*8)*2;body+=rect(x*T+dx,y*T+dy,2+(k%2)*2,2,random()>.4?'#526849':'#293f38');}
  }
  // Fictional blue water and layered stone banks; legal paths cross at drawn bridges.
  const streamX=layout.streamX;
  for(let y=0;y<11;y++){const x=streamX+(y>5?1:0);body+=rect(x*T-4,y*T,34,T,'#223d42')+rect(x*T,y*T,24,T,'#3a7780')+rect(x*T+2,y*T+4,8,2,'#83bbc0')+rect(x*T+12,y*T+14,10,2,'#55959c');}
  for(const key of walk){const [x,y]=key.split(',').map(Number);body+=rect(x*T+3,y*T+3,14,14,'#8a8b65')+rect(x*T+5,y*T+5,10,10,'#b1ad80')+rect(x*T+5,y*T+12,3,2,'#777f5d');}
  // Close path gaps while preserving four-way travel (no diagonal movement).
  for(const key of walk){const [x,y]=key.split(',').map(Number);if(walk.has(`${x+1},${y}`))body+=rect(x*T+13,y*T+4,14,12,'#a3a479');if(walk.has(`${x},${y+1}`))body+=rect(x*T+4,y*T+13,12,14,'#a3a479');}
  for(let y=0;y<11;y++)for(let x=0;x<20;x++){
    // Consume the same random draw at every tile; revealing a route must not
    // reroll distant terrain or the identity of the same expedition map.
    const r=random();
    if(!walk.has(`${x},${y}`)&&!layout.nodes.some(n=>Math.abs(n.x-x)+Math.abs(n.y-y)<2)&&Math.abs(x-streamX)>1){if(r<.24)body+=tree(x*T,y*T-3,r>.12);else if(r<.32)body+=rock(x*T,y*T);}
  }
  return {body,walk:[...walk].sort(),routes};
}
class Screen {
  constructor(name,w,h){this.name=name;this.w=w;this.h=h;this.body=rect(0,0,w,h,P.graphite);this.layers=[];this.bounds=[];}
  art(body){this.body+=body;}
  async text(value,x,y,size=18,bold=false,color=P.ink,maxWidth=999){
    const font=path.join(root,`native/shared/fonts/${bold?'VeraBd':'Vera'}.ttf`);
    const buf=await sharp({text:{text:`<span foreground="${color}">${esc(value)}</span>`,font:`Vera${bold?' Bold':''} ${size}`,fontfile:font,rgba:true}}).png().toBuffer();
    const m=await sharp(buf).metadata();if(m.width>maxWidth)throw new Error(`${this.name}: text '${value}' width ${m.width}>${maxWidth}`);
    if(x<0||y<0||x+m.width>this.w||y+m.height>this.h)throw new Error(`${this.name}: text outside canvas`);
    this.layers.push({input:buf,left:x,top:y});this.bounds.push({value,x,y,width:m.width,height:m.height});
  }
  async asset(name,x,y){const b=images.get(name);if(!b)throw new Error(name);this.layers.push({input:b,left:x,top:y});}
  async export(){
    const definitions=this.layers.map(l=>{return `<image x="${l.left}" y="${l.top}" href="data:image/png;base64,${l.input.toString('base64')}"/>`;}).join('');
    const svg=Buffer.from(`<svg xmlns="http://www.w3.org/2000/svg" width="${this.w}" height="${this.h}" shape-rendering="crispEdges">${this.body}${definitions}</svg>`);
    const png=await sharp(svg).png().toBuffer();
    fs.writeFileSync(path.join(source,`${this.name}.svg`),svg);fs.writeFileSync(path.join(out,`${this.name}.png`),png);
    manifest.screens.push({name:this.name,width:this.w,height:this.h,pngSha256:hash(png),svgSha256:hash(svg)});manifest.textBounds.push({screen:this.name,bounds:this.bounds});
  }
}
async function companionBase(name,mode,title,sub){
  const s=new Screen(name,450,600);s.art(panel(12,12,426,576));
  await s.text(title,29,28,24,true,P.ink,392);await s.text(sub,30,59,15,false,P.muted,390);
  const names=['Probe','Cargo','Companions'];
  for(let i=0;i<3;i++){let x=[28,151,275][i];if(names[i]===mode)s.art(rect(x,98,[91,90,139][i],2,P.cyan));await s.text(names[i],x,82,18,true,names[i]===mode?P.cyan:P.muted);}
  return s;
}
async function resources(s,counts,prep,status=['Paused','Not started','Active']){
  for(let i=0;i<3;i++){
    const x=30+i*133;s.art(rect(x-1,369,124,88,'#19262d'));
    await s.asset(`${['data','energy','essence'][i]}-compact`,x+1,375);
    await s.text(String(counts[i]),x+61,376,27,true,P.ink,58);
    await s.text(['Data','Energy','Essence'][i],x+58,408,14,false,P.muted,65);
    s.art(rect(x+2,442,113,9,'#09141b'));
    s.art(rect(x+3,443,111,7,'#0b181f'));
    if(prep[i]>0)s.art(rect(x+3,443,Math.round(111*prep[i]/100),7,i===2?P.saved:P.cyan));
    await s.text(`${prep[i]}% prep`,x+3,427,14,false,P.muted,114);
    await s.text(status[i],x+3,451,14,false,P.muted,114);
  }
}
async function mapScreen(name,layout,state){
  const s=await companionBase(name,'Probe','Fern valley',state.subtitle);
  s.art(panel(23,107,404,232));
  const g=mapGeometry(layout.seed,layout,state.lead);s.art(`<g transform="translate(25 113)">${g.body}`);
  for(const node of layout.nodes){let status=state.status[node.id]||'unvisited';if(node.id==='cache'&&!state.lead)continue;s.art(marker(node.x*20+10,node.y*20+10,node.type,status));}
  if(!state.lead){const n=layout.nodes.find(n=>n.id==='brook');s.art(rect(n.x*20+28,n.y*20+6,3,3,'#b8bfa2')+rect(n.x*20+35,n.y*20+10,3,3,'#889880')+rect(n.x*20+41,n.y*20+7,3,3,'#889880'));}
  const current=state.tile?{x:state.tile[0],y:state.tile[1]}:layout.nodes.find(n=>n.id===state.position),px=current.x*20+10,py=current.y*20+10;
  s.art(`<path d="M${px} ${py-17}L${px+6} ${py-11}L${px} ${py-5}L${px-6} ${py-11}Z" fill="#f2fbfa" stroke="${P.cyan}" stroke-width="2"/>`);
  if(!state.tile)s.art(`<rect x="${px-15}" y="${py-15}" width="30" height="31" fill="none" stroke="${P.warm}" stroke-width="1"/>`);s.art('</g>');
  for(const n of layout.nodes){if(n.id==='cache'&&!state.lead)continue;const labelX=Math.min(414-n.name.length*8,25+n.x*20-18),labelY=113+n.y*20+23;s.art(rect(labelX-3,labelY-2,n.name.length*8+6,18,'#243a35'));await s.text(n.name,labelX,labelY,14,true,'#e2dec6',110);}
  await s.text(state.activity,30,346,16,true,P.saved,390);await resources(s,state.counts,state.prep,state.prepStatus);
  await s.text(state.place,30,474,20,true,P.ink,390);await s.text(state.purpose,30,504,15,false,P.muted,390);
  await s.text(state.tile?'Directions: move · inspect at places':'Directions: move · Confirm: inspect',30,538,15,false,P.ink,390);
  await s.text('Back: modes',30,560,15,false,P.muted,390);
  await s.export();manifest.maps.push({screen:name,seed:layout.seed,nodes:layout.nodes,routes:layout.routes,legalTiles:g.walk,state});
}
async function detailScreen(name,title,sub,counts,prep,choice,second,kind){
  const s=await companionBase(name,'Probe','Fern valley',sub);s.art(panel(24,110,402,220));
  await s.text(title,42,132,25,true,P.ink,362);
  const layout=JSON.parse(fs.readFileSync(path.join(__dirname,'study.json'))).maps[0];
  const g=mapGeometry(layout.seed,layout,kind==='cache');const node=layout.nodes.find(n=>n.type===kind);
  let local=g.body+marker(node.x*20+10,node.y*20+10,kind,'unvisited');
  if(kind==='brook')local+=rect(178,165,3,3,'#c6d2af')+rect(187,170,3,3,'#c6d2af')+rect(197,165,3,3,'#c6d2af')+rect(205,170,3,3,'#a3b798');
  const crop=kind==='brook'?{x:60,y:137,w:181,h:57}:{x:240,y:77,w:126,h:57};
  s.art(`<svg x="43" y="172" width="${crop.w*2}" height="114" viewBox="${crop.x} ${crop.y} ${crop.w} ${crop.h}">${local}</svg>`);
  if(kind==='cache'){await s.asset('sample-neutral',332,185);await s.text('Sealed',326,253,17,true,P.ink,80);await s.text('Contents unknown · collect to take it',43,300,17,false,P.muted,362);}
  else await s.text('Inspect the markings to find their direction.',43,300,16,false,P.ink,362);
  await s.text(kind==='cache'?'Essence · Moss bend · 2 attempts left':'Data · Camp · 2 attempts left',30,346,16,true,P.saved,390);await resources(s,counts,prep,kind==='cache'?['Paused','Not started','Active']:['Active','Not started','Not started']);
  s.art(focus(29,479,392,30));await s.text(choice,45,487,17,true,P.warm,360);await s.text(second,45,517,17,false,P.ink,360);
  await s.text('Up/Down: choose · Confirm: act',30,540,15,false,P.ink,390);await s.text('Back: same map position',30,560,15,false,P.muted,390);await s.export();
}
async function main(){
  for(const reference of ['design/game-art-proposals/35-vault-composition/18-c-refined.png','design/companion-connected-art/03-gemini-probe-corrected.png','design/companion-connected-art/04-gemini-cargo.png','design/companion-connected-art/07-gemini-lab-arrival-blue.png','native/shared/fonts/Vera.ttf','native/shared/fonts/VeraBd.ttf'])manifest.references.push({path:reference,sha256:hash(fs.readFileSync(path.join(root,reference)))});
  manifest.source={buildSha256:hash(fs.readFileSync(__filename)),statesSha256:hash(fs.readFileSync(path.join(__dirname,'study.json'))),mapFootprint:{width:400,height:220,tile:20},localCrops:[{kind:'brook',x:60,y:137,width:181,height:57,scale:2},{kind:'cache',x:240,y:77,width:126,height:57,scale:2}]};
  for(const name of assets){const f=path.join(root,`design/core-v1-art/exports/${name}.png`),b=fs.readFileSync(f),m=await sharp(b).metadata();images.set(name,b);manifest.assets.push({name,path:path.relative(root,f).replaceAll('\\','/'),sha256:hash(b),width:m.width,height:m.height,scale:1});}
  const data=JSON.parse(fs.readFileSync(path.join(__dirname,'study.json')));
  await mapScreen('01-map-seed-a',data.maps[0],data.initial);
  await detailScreen('02-brook-opportunity','Moss bend','Expedition 014 · Moss bend',[1,0,0],[30,0,0],'Inspect trace','Start Essence gathering','brook');
  await mapScreen('03-active-map-lead',data.maps[0],data.active);
  await detailScreen('04-sealed-sample','Old cache','Located lead · unopened sample',data.active.counts,data.active.prep,'Collect sealed sample','Leave it here','cache');
  await returnScreens(data);
  await mapScreen('07-map-seed-b',data.maps[1],data.initial);
  await labScreens(data);
  await proofSheet('companion-sequence',manifest.screens.slice(0,6).map(s=>s.name),1350,1200,3,450,600);
  await proofSheet('two-seeds',['01-map-seed-a','07-map-seed-b'],900,600,2,450,600);
  await proofSheet('lab-received',['08-lab-received-log','09-lab-received-detail'],1024,1200,1,1024,600);
  fs.writeFileSync(path.join(__dirname,'manifest.json'),JSON.stringify(manifest,null,2)+'\n');
  console.log(JSON.stringify({screens:manifest.screens.map(s=>`${s.name} ${s.width}×${s.height}`),assets:manifest.assets.length}));
}
// Journey review and received Lab projections are authored below from study.json.
async function returnScreens(data){
  const s=await companionBase('05-return-review','Cargo','Return to Lab','014 · review freezes gathering');
  s.art(panel(24,111,402,284));await s.text('Earned this expedition',43,133,22,true,P.ink,362);
  for(let i=0;i<3;i++){const x=42+i*126;await s.asset(`${['data','energy','essence'][i]}-compact`,x+8,173);await s.text(String(data.result.counts[i]),x+63,177,28,true);await s.text(['Data','Energy','Essence'][i],x+10,238,18,false,P.ink,110);}
  s.art(rect(42,270,364,1,'#47606b'));await s.asset('sample-neutral',48,281);await s.text('1 sealed sample',123,291,22,true,P.ink,280);await s.text('Contents unknown',123,324,17,false,P.muted,275);
  await s.text('Supplies 2/4 · Capsules 1/1',43,362,17,false,P.muted,363);
  await s.text('Send earned items + sample.',30,412,18,false,P.ink,390);await s.text('Seals expedition; gathering stops.',30,439,18,false,P.muted,390);
  await s.text('Send to Lab',46,471,19,true,P.ink,359);s.art(focus(29,500,392,34));await s.text('Keep exploring',45,509,18,true,P.warm,360);
  await s.text('Up/Down: choose · Confirm: select',30,540,15,false,P.ink,390);await s.text('Back: keep · same position',30,560,14,false,P.muted,390);await s.export();
  const r=await companionBase('06-sent-awaiting-receipt','Cargo','Expedition sent','014 · awaiting Lab receipt');r.art(panel(24,112,402,347));
  await r.text('Returning record sealed',42,136,23,true,P.ink,361);
  await r.asset('data-primary',47,192);await r.asset('energy-primary',178,192);await r.asset('essence-primary',308,197);
  for(let i=0;i<3;i++){await r.text(String(data.result.counts[i]),[86,217,347][i],302,28,true);await r.text(['Data','Energy','Essence'][i],[58,178,311][i],339,18,false,P.ink,110);}
  await r.asset('sample-neutral',51,386);await r.text('1 sealed sample',128,395,23,true,P.ink,276);await r.text('Gathering stopped',128,428,18,false,P.muted,273);
  await r.text('The Lab has not accepted it yet.',30,481,18,false,P.ink,390);await r.text('No new gathering from this expedition.',30,510,16,false,P.muted,390);
  await r.text('Back: modes · receipt stays pending',30,552,15,false,P.ink,390);await r.export();
}
async function labBase(name){const s=new Screen(name,1024,600);s.art(panel(14,14,996,572));await s.text('Expedition log',36,35,32,true,P.ink,510);await s.text('Received expeditions',38,78,20,false,P.muted,600);return s;}
async function archiveMap(s,layout,x,y){
  const record={...layout,routes:[['camp','brook']],leadRoute:['brook','cache']};
  const g=mapGeometry(record.seed,record,true);let body=g.body;
  for(const id of ['camp','brook','cache']){const n=record.nodes.find(n=>n.id===id);body+=marker(n.x*20+10,n.y*20+10,n.type,id==='cache'?'resolved':'inspected');}
  s.art(`<g transform="translate(${x} ${y})">${body}</g>`);
  for(const id of ['camp','brook','cache']){const n=record.nodes.find(n=>n.id===id),lx=x+n.x*20-18,ly=y+n.y*20+23;s.art(rect(lx-3,ly-2,n.name.length*8+6,18,'#243a35'));await s.text(n.name,lx,ly,14,true,'#e2dec6',110);}
}
async function labScreens(data){
  const s=await labBase('08-lab-received-log');s.art(panel(30,116,353,418));s.art(panel(402,116,592,418));
  await s.text('Received expeditions',50,138,25,true,P.ink,311);s.art(focus(47,185,319,85));await s.text('Fern valley · 014',62,201,23,true,P.warm,288);await s.text('Today · 14:20 · accepted',62,237,17,false,P.ink,288);
  await s.text('Earlier records',52,309,19,true,P.muted,310);await s.text('No earlier expeditions',52,344,18,false,P.muted,310);
  await s.text('Recorded route · 014',425,140,27,true,P.ink,545);await s.text('Received Today · 14:20 · accepted',426,178,19,false,P.saved,545);
  await archiveMap(s,data.maps[0],426,212);await s.asset('sample-neutral',867,250);await s.text('1 sealed',850,332,20,true,P.ink,126);await s.text('sample',850,362,20,true,P.ink,126);await s.text('Research at Lab',843,401,16,false,P.muted,142);
  for(let i=0;i<3;i++){const x=431+i*181;await s.asset(`${['data','energy','essence'][i]}-compact`,x,459);await s.text(`${data.result.counts[i]} ${['Data','Energy','Essence'][i]}`,x+57,478,21,true,P.ink,123);}
  await s.text('Up/Down: choose record · Confirm: details · Back: Home',38,551,18,false,P.ink,945);await s.export();
  const d=await labBase('09-lab-received-detail');d.art(panel(30,116,582,418));d.art(panel(632,116,362,418));await d.text('Fern valley · expedition 014',50,139,27,true,P.ink,540);await d.text('Received Today · 14:20',51,179,19,false,P.saved,540);
  await archiveMap(d,data.maps[0],100,220);await d.text('Moss bend: trace inspected.',51,464,20,false,P.ink,540);await d.text('Old cache: sealed capsule collected.',51,499,20,false,P.ink,540);
  await d.text('Accepted contents',652,141,25,true,P.ink,321);
  for(let i=0;i<3;i++){await d.asset(`${['data','energy','essence'][i]}-compact`,655,195+i*72);await d.text(`${data.result.counts[i]} ${['Data','Energy','Essence'][i]}`,721,208+i*72,24,true,P.ink,250);}
  await d.asset('sample-neutral',654,415);await d.text('1 sealed sample',721,426,20,true,P.ink,250);await d.text('Research at the Lab',653,496,18,false,P.muted,320);
  await d.text('Back: received log',38,551,18,false,P.ink,945);await d.export();
}
async function proofSheet(name,names,width,height,columns,cellWidth,cellHeight){
  const layers=names.map((name,index)=>({input:path.join(out,`${name}.png`),left:(index%columns)*cellWidth,top:Math.floor(index/columns)*cellHeight}));
  const png=await sharp({create:{width,height,channels:4,background:P.graphite}}).composite(layers).png().toBuffer();
  fs.writeFileSync(path.join(out,`${name}.png`),png);manifest.proofs.push({name,width,height,scale:1,screens:names,sha256:hash(png)});
}
main().catch(error=>{console.error(error);process.exitCode=1;});
