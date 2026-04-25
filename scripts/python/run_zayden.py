import subprocess
import socket

def get_local_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(('8.8.8.8', 80))
        ip = s.getsockname()[0]
    except:
        ip = '127.0.0.1'
    finally:
        s.close()
    return ip

print(f"""
╔══════════════════════════════════╗
║  🧠 ZAYDEN ENGINE - SYNC-7 NODE  ║
║  IP: {get_local_ip():<20}║
║  O=9 | φ=1.618                  ║
╚══════════════════════════════════╝
""")

while True:
    user = input("\n📝 You: ").strip()
    if user.lower() in ['exit', 'quit']:
        print("Goodbye")
        break
    if not user:
        continue
    
    print("🤔 Thinking...")
    try:
        result = subprocess.run(
            ["ollama", "run", "phi3:mini", user],
            capture_output=True,
            text=True,
            timeout=30
        )
        print(f"\n🧬 Zayden: {result.stdout.strip()}\n")
    except Exception as e:
        print(f"Error: {e}")
