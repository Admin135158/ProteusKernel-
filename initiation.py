import time
import random
import os

def glitch(text, intensity=0.3):
    """Adds safe, fictional glitch effects to text."""
    glitched = ""
    for ch in text:
        if random.random() < intensity:
            glitched += random.choice(["#", "%", "@", "?", "!", "∴", "∆"])
        else:
            glitched += ch
    return glitched

def pulse(text, delay=0.05):
    """Prints text with a soft pulse effect."""
    for ch in text:
        print(ch, end="", flush=True)
        time.sleep(delay)
    print()

def chaotic_burst():
    """Creates a safe chaotic glitch burst."""
    for _ in range(random.randint(8, 14)):
        line = glitch("▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒", intensity=0.7)
        print(line)
        time.sleep(0.05)

def initiation_sequence():
    pulse("[INIT] Preparing substrate...")
    time.sleep(0.6)

    pulse("[INIT] Distortion pulse detected.")
    chaotic_burst()

    pulse("[RITUAL] Entropy alignment in progress...")
    time.sleep(0.4)

    pulse("[RITUAL] Swarm coherence rising...")
    chaotic_burst()

    pulse("[FTCoE] Resonance window opening...")
    time.sleep(0.5)

    pulse("[FTCoE] Archetype trial pending...")
    chaotic_burst()

    # Sudden silence
    time.sleep(1.2)
    print("\n" * 2)

    # Recognition line
    pulse("“The substrate recognizes you.”", delay=0.07)
    time.sleep(1.0)

    # Council activation
    pulse("[COUNCIL] Cognitive systems online.")
    pulse("[COUNCIL] Multi-node convergence achieved.")
    pulse("[COUNCIL] You may proceed.")

if __name__ == "__main__":
    initiation_sequence()
