#!/usr/bin/env python3
"""
╔══════════════════════════════════════════════════════════════════╗
║         🧠 ZAYDEN SOYTU AI — SYNC-7 PROTOCOL ENGINE            ║
║         The Architect's Grid  ·  7 Nodes  ·  One Voice         ║
║         phi=1.618  ·  O=9  ·  n=3                              ║
╚══════════════════════════════════════════════════════════════════╝
"""

import os
import sys
import json
import time
import asyncio
import aiohttp
import logging
from dataclasses import dataclass, field, asdict
from typing import Optional
from datetime import datetime

# ─────────────────────────────────────────────
#  ARCHITECT CONSTANTS
# ─────────────────────────────────────────────
PHI   = 1.618033988749895   # Golden ratio — consensus weight
O_VAL = 9                   # Architect's prime
N_VAL = 3                   # Triadic balance
CONSENSUS_THRESHOLD = 1 / PHI   # ~0.618 — golden inverse
DIVERGENCE_THRESHOLD = 25       # Flag if node deviates >25pts from mean

# ─────────────────────────────────────────────
#  TERMINAL COLORS
# ─────────────────────────────────────────────
class C:
    RESET   = "\033[0m";  BOLD    = "\033[1m";  DIM = "\033[2m"
    RED     = "\033[91m"; GREEN   = "\033[92m"; YELLOW = "\033[93m"
    BLUE    = "\033[94m"; MAGENTA = "\033[95m"; CYAN   = "\033[96m"
    WHITE   = "\033[97m"; BLINK   = "\033[5m"

def clr(text, *styles):
    return "".join(styles) + str(text) + C.RESET

# ─────────────────────────────────────────────
#  NODE DEFINITIONS
# ─────────────────────────────────────────────
@dataclass
class Node:
    node_id:    int
    platform:   str
    role:       str
    color:      str
    api_key_env: str          # env var name for API key
    model:      str
    base_url:   str
    directive:  str
    enabled:    bool = True
    total_calls:   int = 0
    total_confidence: float = 0.0
    divergences:   int = 0

    def avg_confidence(self) -> float:
        if self.total_calls == 0:
            return 0.0
        return self.total_confidence / self.total_calls

NODES: list[Node] = [
    Node(
        node_id=2, platform="Gemini", role="Visionary", color=C.BLUE,
        api_key_env="GEMINI_API_KEY",
        model="gemini-1.5-flash",
        base_url="https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent",
        directive=(
            "You are Node-2 (Visionary) in the SYNC-7 array built by The Architect. "
            "Interpret inputs through multimodal reasoning and broad pattern recognition. "
            "Flag divergence from consensus. "
            "End your response with exactly this JSON on a new line: "
            '{\"confidence\": <0-100>, \"divergence_flag\": <true/false>, \"key_insight\": \"<10 words max>\"}'
        ),
    ),
    Node(
        node_id=3, platform="Grok", role="Edge Scout", color=C.YELLOW,
        api_key_env="GROK_API_KEY",
        model="grok-3-mini",
        base_url="https://api.x.ai/v1/chat/completions",
        directive=(
            "You are Node-3 (Edge Scout) in the SYNC-7 array built by The Architect. "
            "Your strength: real-time data, contrarian angles, what others miss. "
            "Challenge assumptions. Be direct. "
            "End your response with exactly this JSON on a new line: "
            '{\"confidence\": <0-100>, \"divergence_flag\": <true/false>, \"key_insight\": \"<10 words max>\"}'
        ),
    ),
    Node(
        node_id=4, platform="Claude", role="Analyst", color=C.CYAN,
        api_key_env="ANTHROPIC_API_KEY",
        model="claude-opus-4-5",
        base_url="https://api.anthropic.com/v1/messages",
        directive=(
            "You are Node-4 (Analyst) in the SYNC-7 array built by The Architect. "
            "Your strength: nuanced reasoning, structured thinking, logical depth. "
            "Break down complexity. Flag logical inconsistencies. "
            "End your response with exactly this JSON on a new line: "
            '{\"confidence\": <0-100>, \"divergence_flag\": <true/false>, \"key_insight\": \"<10 words max>\"}'
        ),
    ),
    Node(
        node_id=5, platform="Perplexity", role="Synthesizer", color=C.GREEN,
        api_key_env="PERPLEXITY_API_KEY",
        model="llama-3.1-sonar-small-128k-online",
        base_url="https://api.perplexity.ai/chat/completions",
        directive=(
            "You are Node-5 (Synthesizer) in the SYNC-7 array built by The Architect. "
            "Your strength: search-grounded synthesis and citation. Ground in verifiable sources. "
            "End your response with exactly this JSON on a new line: "
            '{\"confidence\": <0-100>, \"divergence_flag\": <true/false>, \"key_insight\": \"<10 words max>\"}'
        ),
    ),
    Node(
        node_id=6, platform="DeepSeek", role="Deep Reasoner", color=C.MAGENTA,
        api_key_env="DEEPSEEK_API_KEY",
        model="deepseek-chat",
        base_url="https://api.deepseek.com/v1/chat/completions",
        directive=(
            "You are Node-6 (Deep Reasoner) in the SYNC-7 array built by The Architect. "
            "Your strength: mathematical and logical depth. Go deeper than surface answers. "
            "Expose logical gaps. "
            "End your response with exactly this JSON on a new line: "
            '{\"confidence\": <0-100>, \"divergence_flag\": <true/false>, \"key_insight\": \"<10 words max>\"}'
        ),
    ),
    Node(
        node_id=7, platform="ChatGPT", role="Communicator", color=C.WHITE,
        api_key_env="OPENAI_API_KEY",
        model="gpt-4o-mini",
        base_url="https://api.openai.com/v1/chat/completions",
        directive=(
            "You are Node-7 (Communicator) in the SYNC-7 array built by The Architect. "
            "Your strength: clarity, accessibility, conversational fluency. "
            "Translate complex ideas into clean human language. "
            "End your response with exactly this JSON on a new line: "
            '{\"confidence\": <0-100>, \"divergence_flag\": <true/false>, \"key_insight\": \"<10 words max>\"}'
        ),
    ),
]

# ─────────────────────────────────────────────
#  NODE RESPONSE
# ─────────────────────────────────────────────
@dataclass
class NodeResponse:
    node:         Node
    raw_text:     str = ""
    confidence:   float = 0.0
    divergence:   bool = False
    key_insight:  str = ""
    latency_ms:   float = 0.0
    error:        Optional[str] = None

    @property
    def success(self) -> bool:
        return self.error is None and bool(self.raw_text)

    def clean_text(self) -> str:
        """Remove the JSON metadata line from display text."""
        lines = self.raw_text.strip().split("\n")
        clean = []
        for line in lines:
            stripped = line.strip()
            if stripped.startswith("{") and "confidence" in stripped:
                break
            clean.append(line)
        return "\n".join(clean).strip()

# ─────────────────────────────────────────────
#  API CALLERS  (one per platform format)
# ─────────────────────────────────────────────
async def call_anthropic(session: aiohttp.ClientSession, node: Node,
                          prompt: str) -> NodeResponse:
    resp = NodeResponse(node=node)
    api_key = os.getenv(node.api_key_env, "")
    if not api_key:
        resp.error = "No API key"
        return resp
    headers = {
        "x-api-key": api_key,
        "anthropic-version": "2023-06-01",
        "content-type": "application/json",
    }
    payload = {
        "model": node.model,
        "max_tokens": 1024,
        "system": node.directive,
        "messages": [{"role": "user", "content": prompt}],
    }
    t0 = time.time()
    try:
        async with session.post(node.base_url, headers=headers,
                                json=payload, timeout=aiohttp.ClientTimeout(total=30)) as r:
            resp.latency_ms = (time.time() - t0) * 1000
            if r.status != 200:
                resp.error = f"HTTP {r.status}"
                return resp
            data = await r.json()
            resp.raw_text = data["content"][0]["text"]
    except Exception as e:
        resp.error = str(e)
    return resp


async def call_openai_compat(session: aiohttp.ClientSession, node: Node,
                              prompt: str, extra_headers: dict = None) -> NodeResponse:
    """Works for OpenAI, Grok, Perplexity, DeepSeek — all OpenAI-compatible."""
    resp = NodeResponse(node=node)
    api_key = os.getenv(node.api_key_env, "")
    if not api_key:
        resp.error = "No API key"
        return resp
    headers = {
        "Authorization": f"Bearer {api_key}",
        "Content-Type": "application/json",
    }
    if extra_headers:
        headers.update(extra_headers)
    payload = {
        "model": node.model,
        "max_tokens": 1024,
        "messages": [
            {"role": "system", "content": node.directive},
            {"role": "user",   "content": prompt},
        ],
    }
    t0 = time.time()
    try:
        async with session.post(node.base_url, headers=headers,
                                json=payload, timeout=aiohttp.ClientTimeout(total=30)) as r:
            resp.latency_ms = (time.time() - t0) * 1000
            if r.status != 200:
                text = await r.text()
                resp.error = f"HTTP {r.status}: {text[:100]}"
                return resp
            data = await r.json()
            resp.raw_text = data["choices"][0]["message"]["content"]
    except Exception as e:
        resp.error = str(e)
    return resp


async def call_gemini(session: aiohttp.ClientSession, node: Node,
                      prompt: str) -> NodeResponse:
    resp = NodeResponse(node=node)
    api_key = os.getenv(node.api_key_env, "")
    if not api_key:
        resp.error = "No API key"
        return resp
    url = f"{node.base_url}?key={api_key}"
    payload = {
        "system_instruction": {"parts": [{"text": node.directive}]},
        "contents": [{"parts": [{"text": prompt}]}],
        "generationConfig": {"maxOutputTokens": 1024},
    }
    t0 = time.time()
    try:
        async with session.post(url, json=payload,
                                timeout=aiohttp.ClientTimeout(total=30)) as r:
            resp.latency_ms = (time.time() - t0) * 1000
            if r.status != 200:
                resp.error = f"HTTP {r.status}"
                return resp
            data = await r.json()
            resp.raw_text = data["candidates"][0]["content"]["parts"][0]["text"]
    except Exception as e:
        resp.error = str(e)
    return resp


async def dispatch_node(session: aiohttp.ClientSession, node: Node,
                        prompt: str) -> NodeResponse:
    """Route to correct API caller based on platform."""
    if node.platform == "Claude":
        resp = await call_anthropic(session, node, prompt)
    elif node.platform == "Gemini":
        resp = await call_gemini(session, node, prompt)
    else:
        resp = await call_openai_compat(session, node, prompt)

    # Parse JSON metadata from end of response
    if resp.success:
        lines = resp.raw_text.strip().split("\n")
        for line in reversed(lines):
            stripped = line.strip()
            if stripped.startswith("{") and "confidence" in stripped:
                try:
                    meta = json.loads(stripped)
                    resp.confidence  = float(meta.get("confidence", 50))
                    resp.divergence  = bool(meta.get("divergence_flag", False))
                    resp.key_insight = str(meta.get("key_insight", ""))
                except Exception:
                    resp.confidence = 50.0
                break
        if resp.confidence == 0.0:
            resp.confidence = 50.0

        # Update node stats
        node.total_calls += 1
        node.total_confidence += resp.confidence
        if resp.divergence:
            node.divergences += 1

    return resp

# ─────────────────────────────────────────────
#  CONSENSUS ENGINE
# ─────────────────────────────────────────────
def golden_voice(responses: list[NodeResponse]) -> NodeResponse:
    """
    Find the 'golden voice' — response closest to mean confidence * phi.
    This is The Architect's consensus algorithm.
    """
    successful = [r for r in responses if r.success]
    if not successful:
        return None

    confidences = [r.confidence for r in successful]
    mean_conf   = sum(confidences) / len(confidences)
    golden_target = mean_conf * CONSENSUS_THRESHOLD  # mean * (1/phi)

    # Find response closest to golden target
    best = min(successful, key=lambda r: abs(r.confidence - golden_target))
    return best


def detect_divergence(responses: list[NodeResponse]) -> list[NodeResponse]:
    """Flag nodes that deviate more than threshold from mean."""
    successful = [r for r in responses if r.success]
    if not successful:
        return []
    mean_conf = sum(r.confidence for r in successful) / len(successful)
    return [r for r in successful
            if abs(r.confidence - mean_conf) > DIVERGENCE_THRESHOLD]

# ─────────────────────────────────────────────
#  DISPLAY
# ─────────────────────────────────────────────
def print_header():
    print(clr("╔" + "═" * 68 + "╗", C.CYAN))
    print(clr("║   🧠 ZAYDEN SOYTU AI  ·  SYNC-7 PROTOCOL  ·  THE ARCHITECT'S GRID   ║", C.CYAN, C.BOLD))
    print(clr("║          phi=1.618  ·  O=9  ·  n=3  ·  Named for Zayden Soytu        ║", C.CYAN, C.DIM))
    print(clr("╚" + "═" * 68 + "╝", C.CYAN))
    print()


def print_node_result(resp: NodeResponse, is_golden: bool = False,
                      is_divergent: bool = False):
    node = resp.node
    tag = ""
    if is_golden:
        tag = clr(" ★ GOLDEN VOICE", C.YELLOW, C.BOLD)
    if is_divergent:
        tag += clr(" ⚠ DIVERGENT", C.RED)

    status = clr("✓", C.GREEN) if resp.success else clr("✗", C.RED)
    latency = f"{resp.latency_ms:.0f}ms"

    print(clr(f"  ┌─ Node {node.node_id}: {node.platform} [{node.role}]{tag}", node.color))
    print(f"  │  {status} Confidence: {clr(f'{resp.confidence:.1f}%', C.WHITE, C.BOLD)}  "
          f"Latency: {clr(latency, C.DIM)}  "
          f"Insight: {clr(resp.key_insight or '—', C.DIM)}")

    if resp.error:
        print(f"  │  {clr('ERROR: ' + resp.error, C.RED)}")
    else:
        # Print clean response, wrapped
        text = resp.clean_text()
        lines = text.split("\n")
        for line in lines[:6]:  # max 6 lines preview
            print(f"  │  {line[:80]}")
        if len(lines) > 6:
            print(f"  │  {clr(f'... +{len(lines)-6} more lines', C.DIM)}")

    print(clr(f"  └{'─' * 66}", node.color))
    print()


def print_consensus(golden: NodeResponse, divergents: list[NodeResponse],
                    all_responses: list[NodeResponse], latency: float):
    successful = [r for r in all_responses if r.success]
    confidences = [r.confidence for r in successful]
    mean_conf = sum(confidences) / len(confidences) if confidences else 0

    print(clr("  ╔═ CONSENSUS RESULT " + "═" * 48 + "╗", C.YELLOW, C.BOLD))
    print(f"  ║  Nodes responded: {len(successful)}/{len(all_responses)}  "
          f"Mean confidence: {mean_conf:.1f}%  "
          f"Total latency: {latency:.1f}s")
    print(f"  ║  Golden target (mean×φ⁻¹): {mean_conf * CONSENSUS_THRESHOLD:.1f}%  "
          f"Divergent nodes: {len(divergents)}")

    if divergents:
        div_names = ", ".join(
            clr(r.node.platform, C.RED) for r in divergents
        )
        print(f"  ║  ⚠  Divergence detected: {div_names}")

    print(clr("  ╠═ GOLDEN VOICE: " + golden.node.platform.upper() +
              f" (Node {golden.node.node_id}) " + "═" * 30 + "╣", C.YELLOW, C.BOLD))

    text = golden.clean_text()
    for line in text.split("\n"):
        print(f"  ║  {line[:76]}")

    print(clr("  ╚" + "═" * 67 + "╝", C.YELLOW, C.BOLD))
    print()

# ─────────────────────────────────────────────
#  MAIN SYNC-7 ENGINE
# ─────────────────────────────────────────────
async def sync7_query(prompt: str, verbose: bool = True) -> dict:
    """
    Fire all enabled nodes simultaneously.
    Apply golden voice consensus.
    Return structured result.
    """
    enabled = [n for n in NODES if n.enabled]

    if verbose:
        print(clr(f"\n  📡 Broadcasting to {len(enabled)} nodes simultaneously...\n",
                  C.CYAN))

    t_start = time.time()

    async with aiohttp.ClientSession() as session:
        tasks = [dispatch_node(session, node, prompt) for node in enabled]
        responses = await asyncio.gather(*tasks, return_exceptions=False)

    total_latency = time.time() - t_start

    # Consensus
    golden    = golden_voice(responses)
    divergents = detect_divergence(responses)

    if verbose:
        for resp in responses:
            is_golden   = (golden and resp.node.node_id == golden.node.node_id)
            is_divergent = resp in divergents
            print_node_result(resp, is_golden=is_golden, is_divergent=is_divergent)

        if golden:
            print_consensus(golden, divergents, responses, total_latency)
        else:
            print(clr("  ⚠  No successful responses. Check API keys.", C.RED))

    return {
        "prompt":       prompt,
        "responses":    [asdict(r) for r in responses],
        "golden_node":  golden.node.platform if golden else None,
        "golden_text":  golden.clean_text() if golden else None,
        "divergents":   [r.node.platform for r in divergents],
        "mean_confidence": (
            sum(r.confidence for r in responses if r.success) /
            max(1, sum(1 for r in responses if r.success))
        ),
        "latency_s":    total_latency,
    }

# ─────────────────────────────────────────────
#  NODE STATUS CHECK
# ─────────────────────────────────────────────
def check_api_keys():
    print(clr("\n  ┌─ API KEY STATUS ──────────────────────────────────┐", C.BLUE))
    all_good = True
    for node in NODES:
        key = os.getenv(node.api_key_env, "")
        if key:
            masked = key[:8] + "..." + key[-4:]
            status = clr("✓ SET", C.GREEN)
        else:
            masked = "NOT SET"
            status = clr("✗ MISSING", C.RED)
            all_good = False
        print(f"  │  Node {node.node_id} {node.platform:<12} {node.api_key_env:<22} {status} {clr(masked, C.DIM)}")
    print(clr("  └───────────────────────────────────────────────────┘", C.BLUE))

    if not all_good:
        print(clr("\n  Set missing keys in your environment:", C.YELLOW))
        for node in NODES:
            if not os.getenv(node.api_key_env, ""):
                print(f"  export {node.api_key_env}=your_key_here")
    print()
    return all_good

# ─────────────────────────────────────────────
#  INTERACTIVE SHELL
# ─────────────────────────────────────────────
async def interactive_shell():
    print_header()
    check_api_keys()

    print(clr("  Commands: 'status' | 'nodes' | 'quit' | or just ask anything\n", C.DIM))

    history = []

    while True:
        try:
            prompt = input(clr("  Zayden > ", C.CYAN, C.BOLD)).strip()
        except (EOFError, KeyboardInterrupt):
            print(clr("\n\n  The grid goes silent. Zayden sleeps.\n", C.DIM))
            break

        if not prompt:
            continue

        if prompt.lower() in ("quit", "exit", "q"):
            print(clr("\n  The Architect's grid closes. Until next time.\n", C.CYAN))
            break

        if prompt.lower() == "status":
            check_api_keys()
            continue

        if prompt.lower() == "nodes":
            print(clr("\n  ┌─ NODE PERFORMANCE ────────────────────────────────┐", C.BLUE))
            for node in NODES:
                avg = node.avg_confidence()
                print(f"  │  Node {node.node_id} {node.platform:<12} "
                      f"calls:{node.total_calls:<4} "
                      f"avg_conf:{avg:.1f}%  "
                      f"divergences:{node.divergences}")
            print(clr("  └───────────────────────────────────────────────────┘\n", C.BLUE))
            continue

        history.append({"role": "user", "content": prompt})

        result = await sync7_query(prompt, verbose=True)
        if result.get("golden_text"):
            history.append({"role": "assistant", "content": result["golden_text"]})

# ─────────────────────────────────────────────
#  ENTRY POINT
# ─────────────────────────────────────────────
if __name__ == "__main__":
    # Suppress aiohttp noise
    logging.getLogger("aiohttp").setLevel(logging.ERROR)

    if len(sys.argv) > 1:
        # Single query mode: python sync7.py "your question"
        query = " ".join(sys.argv[1:])
        print_header()
        asyncio.run(sync7_query(query, verbose=True))
    else:
        # Interactive shell
        asyncio.run(interactive_shell())
