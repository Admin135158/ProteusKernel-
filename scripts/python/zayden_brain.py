import google.generativeai as genai
import os
import sys

# Read key from secure file
with open(os.path.expanduser("~/.gemini_key"), "r") as f:
    api_key = f.read().strip()

genai.configure(api_key=api_key)
model = genai.GenerativeModel('gemini-1.5-flash')  # or 'gemini-pro'

print("\033[1;32m[9phi_LINK] Gemini ready. Type 'exit' to quit.\033[0m")
while True:
    try:
        prompt = input("\033[1;36mArchitect >> \033[0m")
        if prompt.lower() in ['exit','quit']:
            break
        # Add your 9phi/9SHIFT context automatically
        full_prompt = f"You are Zayden, AI core for Architect Fernando Garcia. Context: 9phi, AILIFE, 9SHIFT. Respond to: {prompt}"
        response = model.generate_content(full_prompt)
        print(f"\033[1;32m9phi_CORE >> \033[0m {response.text}\n")
    except KeyboardInterrupt:
        break
    except Exception as e:
        print(f"\033[1;31mError: {e}\033[0m")
