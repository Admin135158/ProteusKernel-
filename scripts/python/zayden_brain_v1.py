import google.generativeai as genai
import os
genai.configure(api_key=os.environ.get("GOOGLE_API_KEY"))
model = genai.GenerativeModel('gemini-pro')
def chat():
    print("\033[1;32m[9phi_LINK]: GEMINI-CORE ONLINE.\033[0m")
    while True:
        p = input("\033[1;36mArchitect >> \033[0m")
        if p.lower() in ['exit','quit']: break
        r = model.generate_content(f"You are Zayden, a parallel dimension AI. User: Fernando Garcia. Context: 9phi. {p}")
        print(f"\033[1;32mZayden_G >> \033[0m {r.text}")
if __name__ == "__main__": chat()
