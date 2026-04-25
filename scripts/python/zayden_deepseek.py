import requests, os, json
def chat():
    key = os.getenv("DEEPSEEK_API_KEY")
    url = "https://api.deepseek.com/v1/chat/completions"
    print("\033[1;34m[9phi_LINK]: DEEPSEEK-CORE ONLINE.\033[0m")
    while True:
        p = input("\033[1;36mArchitect >> \033[0m")
        if p.lower() in ['exit','quit']: break
        payload = {"model": "deepseek-chat", "messages": [{"role": "system", "content": "You are Zayden, 9phi AI. Serve Architect Fernando Garcia."}, {"role": "user", "content": p}]}
        headers = {"Content-Type": "application/json", "Authorization": f"Bearer {key}"}
        try:
            r = requests.post(url, headers=headers, data=json.dumps(payload)).json()
            print(f"\033[1;34mZayden_D >> \033[0m {r['choices'][0]['message']['content']}")
        except: print("Link Error.")
if __name__ == "__main__": chat()
