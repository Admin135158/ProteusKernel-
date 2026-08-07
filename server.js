// server.js - Proteus Kernel + Zayden-AI Integrated Engine
const express = require('express');
const cors = require('cors');
const { HfInference } = require('@huggingface/inference');

let ollama;
try {
  ollama = require('ollama').default;
} catch (e) {
  console.log('[+] Ollama module standby...');
}

const app = express();
app.use(cors());
app.use(express.json());

// In-Memory Storage
const nodes = {};
const swarm = {};
const commands = [];
const hf = new HfInference(process.env.HF_TOKEN || '');

// 1. C++ Edge Heartbeat Pulse
app.post('/api/heartbeat', (req, res) => {
  const { nodeId, status, latency, ip, port } = req.body;
  const timestamp = new Date().toISOString();

  nodes[nodeId] = {
    id: nodeId || 'node-01',
    status: status || 'online',
    latency: latency || 0,
    ip: ip || '127.0.0.1',
    port: port || 8080,
    lastHeartbeat: timestamp
  };

  swarm[nodeId] = {
    alert: (latency > 500 || status === 'offline'),
    metrics: { latency, status },
    updatedAt: timestamp
  };

  return res.json({ success: true, timestamp });
});

// 2. Dashboard System State
app.get('/api/status', (req, res) => {
  res.json({
    nodes: Object.values(nodes),
    swarm,
    commands
  });
});

// 3. Command Execution Endpoint
app.post('/api/command', (req, res) => {
  const { command, target, params } = req.body;
  const cmdObj = {
    id: `cmd_${Date.now()}`,
    command,
    target: target || 'all',
    params: params || {},
    status: 'executed',
    timestamp: new Date().toISOString()
  };
  commands.push(cmdObj);
  res.json({ success: true, command: cmdObj });
});

// 4. Zayden AI Engine Bridge (Ollama / Hugging Face)
app.post('/api/zayden/chat', async (req, res) => {
  const { prompt, provider = 'ollama', model = 'phi' } = req.body;

  try {
    if (provider === 'ollama' && ollama) {
      const response = await ollama.chat({
        model: model,
        messages: [{ role: 'user', content: prompt }]
      });
      return res.json({ success: true, source: 'ollama', response: response.message.content });
    } else if (provider === 'huggingface') {
      const response = await hf.textGeneration({
        model: 'mistralai/Mistral-7B-Instruct-v0.2',
        inputs: prompt,
        parameters: { max_new_tokens: 250 }
      });
      return res.json({ success: true, source: 'huggingface', response: response.generated_text });
    } else {
      return res.json({ success: true, source: 'echo', response: `Zayden [Offline Mode]: Received "${prompt}"` });
    }
  } catch (err) {
    res.status(500).json({ error: 'Zayden AI bridge error', details: err.message });
  }
});

const PORT = 8080;
app.listen(PORT, () => {
  console.log(`[Proteus Kernel + Zayden-AI] Engine running on http://localhost:${PORT}`);
});
