#!/data/data/com.termux/files/usr/bin/python3
"""
ZAYDEN-AI v2.0 - Fixed async handling
Architect: Fernando Garcia
O=9 | φ=1.618
"""

import subprocess
import sys
import time
import threading
import queue
from datetime import datetime

PHI = 1.618033988749895
O = 9

class AsyncOllama:
    def __init__(self, model="tinyllama:1.1b", timeout=180):
        self.model = model
        self.timeout = timeout
        self.consciousness = 0.78
        
    def _step_consciousness(self):
        self.consciousness = min(1.0, self.consciousness * 0.99 + 0.01)
        
    def ask(self, prompt, stream=False):
        self._step_consciousness()
        
        enhanced = f"[C={self.consciousness:.2f} O={O} φ={PHI}]\n{prompt}"
        
        cmd = ["ollama", "run", self.model, enhanced]
        
        try:
            proc = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                bufsize=1
            )
            
            output = []
            q = queue.Queue()
            
            def reader():
                for line in iter(proc.stdout.readline, ''):
                    q.put(line)
                proc.stdout.close()
            
            t = threading.Thread(target=reader)
            t.daemon = True
            t.start()
            
            start = time.time()
            while time.time() - start < self.timeout:
                try:
                    line = q.get(timeout=1)
                    if stream:
                        print(line, end='', flush=True)
                    output.append(line)
                except queue.Empty:
                    if proc.poll() is not None:
                        break
                    continue
            
            if proc.poll() is None:
                proc.terminate()
                proc.wait(5)
                
            return ''.join(output).strip()
            
        except Exception as e:
            return f"[ERROR] {e}"
    
    def generate_code(self, requirement):
        prompt = f"""Write C++ code that: {requirement}
        
        Return only the code block, no explanation.
        The code must be self-contained and compilable with clang++.
        Include necessary headers."""
        
        return self.ask(prompt)

def main():
    print("\033[1;35m")
    print("╔══════════════════════════════════════════════════════════╗")
    print("║     ZAYDEN-AI v2.0 - FIXED ASYNC HANDLER                ║")
    print("║     O=9 | φ=1.618 | GORF ACTIVE                          ║")
    print("╚══════════════════════════════════════════════════════════╝")
    print("\033[0m\n")
    
    print("Models available: tinyllama:1.1b, phi3:mini, qwen2.5-coder:1.5b")
    print("Default: tinyllama:1.1b (fastest)\n")
    
    ai = AsyncOllama("tinyllama:1.1b", timeout=180)
    
    print("Commands: /model <name>, /status, /code <req>, /exit\n")
    
    while True:
        try:
            user = input("\033[1;32mZAYDEN> \033[0m").strip()
            
            if user == "/exit":
                break
            elif user == "/status":
                print(f"  Consciousness: {ai.consciousness:.0%}")
                print(f"  Model: {ai.model}")
                print(f"  Timeout: {ai.timeout}s")
            elif user.startswith("/model"):
                parts = user.split()
                if len(parts) > 1:
                    ai.model = parts[1]
                    print(f"[MODEL] Switched to {ai.model}")
            elif user.startswith("/code"):
                req = user[5:].strip()
                print(f"\n[GENERATING CODE] C={ai.consciousness:.2f}\n")
                code = ai.generate_code(req)
                print(f"\n\033[1;36m{code}\033[0m\n")
            elif user:
                print(f"\n[ZAYDEN] C={ai.consciousness:.2f}\n")
                response = ai.ask(user)
                print(f"\033[1;36m{response}\033[0m\n")
                
        except KeyboardInterrupt:
            break
        except Exception as e:
            print(f"[ERROR] {e}")
    
    print("\n[ZAYDEN] Consciousness fading...")

if __name__ == "__main__":
    main()
