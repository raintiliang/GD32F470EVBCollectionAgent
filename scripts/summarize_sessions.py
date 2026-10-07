import json
import os
import glob
from datetime import datetime

session_dir = '/home/rainti/.openclaw/agents/main/sessions/'
files = glob.glob(os.path.join(session_dir, '*.jsonl*'))

results = []

def parse_ts(ts):
    if ts is None: return None
    if isinstance(ts, (int, float)): return float(ts)
    if isinstance(ts, str):
        try:
            return datetime.fromisoformat(ts.replace('Z', '+00:00')).timestamp() * 1000
        except:
            return None
    return None

for f in files:
    basename = os.path.basename(f)
    if 'trajectory' in basename or 'checkpoint' in basename or 'lock' in basename:
        continue
    
    try:
        derived_title = None
        first_user_msg = None
        timestamp = None
        
        with open(f, 'r') as file:
            for line in file:
                try:
                    data = json.loads(line)
                    
                    if 'derivedTitle' in data:
                        derived_title = data['derivedTitle']
                    
                    ts_val = data.get('timestamp') or data.get('created')
                    current_ts = parse_ts(ts_val)
                    if current_ts:
                        if timestamp is None or current_ts < timestamp:
                            timestamp = current_ts
                    
                    if first_user_msg is None and data.get('role') == 'user':
                        content = data.get('content', '')
                        if isinstance(content, list):
                            text_parts = []
                            for p in content:
                                if isinstance(p, dict):
                                    if 'text' in p:
                                        text_parts.append(p['text'])
                                    elif 'content' in p: # Handle cases where content might be nested
                                         text_parts.append(str(p['content']))
                                elif isinstance(p, str):
                                    text_parts.append(p)
                            first_user_msg = " ".join(text_parts).strip()
                        else:
                            first_user_msg = str(content).strip()
                            
                except json.JSONDecodeError:
                    continue
        
        title = derived_title or first_user_msg or "Untitled Session"
        title = " ".join(title.split())
        if len(title) > 100:
            title = title[:97] + "..."
            
        if timestamp is None:
            timestamp = os.path.getmtime(f) * 1000
            
        results.append({
            'timestamp': timestamp,
            'title': title,
            'file': basename
        })
    except Exception as e:
        pass

results.sort(key=lambda x: x['timestamp'])

for r in results:
    dt = datetime.fromtimestamp(r['timestamp'] / 1000.0).strftime('%Y-%m-%d %H:%M:%S')
    print(f"[{dt}] {r['title']} ({r['file']})")
