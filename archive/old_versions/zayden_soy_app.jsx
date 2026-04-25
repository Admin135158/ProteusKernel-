import React, { useState, useRef, useEffect } from 'react';
import { Send, Home, MessageSquare, Zap, Brain, TrendingUp } from 'lucide-react';

export default function ZaydenSoyApp() {
  const [currentScreen, setCurrentScreen] = useState('home');
  const [messages, setMessages] = useState([
    {
      id: 1,
      sender: 'jarvis',
      text: 'Good day. I am Jarvis, your AI assistant powered by Zayden Soy Intelligence. I synthesize insights from seven specialized nodes to provide reliable guidance on predictions and consciousness. How may I assist you?',
      confidence: 95,
      timestamp: '8:31 AM'
    }
  ]);
  const [inputValue, setInputValue] = useState('');
  const messagesEndRef = useRef(null);

  const scrollToBottom = () => {
    messagesEndRef.current?.scrollIntoView({ behavior: 'smooth' });
  };

  useEffect(() => {
    scrollToBottom();
  }, [messages]);

  const handleSendMessage = () => {
    if (inputValue.trim()) {
      // Add user message
      const userMessage = {
        id: messages.length + 1,
        sender: 'user',
        text: inputValue,
        timestamp: new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })
      };
      setMessages([...messages, userMessage]);

      // Simulate Jarvis response
      setTimeout(() => {
        const responses = [
          'I\'m analyzing your query across all seven consciousness nodes. Based on current system parameters, this prediction carries high confidence.',
          'Interesting inquiry. The intelligence engine is processing multiple prediction vectors simultaneously. Your insight has been registered.',
          'This aligns with current consciousness state matrices. I recommend reviewing the full predictions dashboard for comprehensive analysis.',
          'Processing complete. The prediction engine suggests monitoring this metric closely over the next 24 hours.'
        ];
        const randomResponse = responses[Math.floor(Math.random() * responses.length)];
        const jarvisMessage = {
          id: messages.length + 2,
          sender: 'jarvis',
          text: randomResponse,
          confidence: 85 + Math.random() * 15,
          timestamp: new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })
        };
        setMessages(prev => [...prev, jarvisMessage]);
      }, 800);

      setInputValue('');
    }
  };

  const HomeScreen = () => (
    <div className="min-h-screen bg-gradient-to-br from-slate-900 via-slate-800 to-slate-900 pb-24">
      {/* Header */}
      <div className="pt-8 px-6 mb-8">
        <div className="flex items-baseline gap-2">
          <h1 className="text-4xl font-black text-white tracking-tight">Zayden Soy</h1>
        </div>
        <p className="text-slate-400 mt-1 text-sm font-medium">Intelligence - AI Prediction Engine</p>
      </div>

      {/* Consciousness Level Card */}
      <div className="px-6 mb-6">
        <div className="bg-gradient-to-br from-slate-800 to-slate-700 rounded-2xl p-8 border border-slate-700/50 shadow-2xl">
          <h2 className="text-slate-300 text-lg font-semibold mb-8">Consciousness Level</h2>
          
          <div className="flex justify-center mb-8">
            <div className="relative w-48 h-48">
              <svg className="w-full h-full transform -rotate-90" viewBox="0 0 200 200">
                <circle
                  cx="100"
                  cy="100"
                  r="90"
                  fill="none"
                  stroke="#1e293b"
                  strokeWidth="8"
                />
                <circle
                  cx="100"
                  cy="100"
                  r="90"
                  fill="none"
                  stroke="url(#gradient)"
                  strokeWidth="8"
                  strokeDasharray={`${2 * Math.PI * 90 * 0.72} ${2 * Math.PI * 90}`}
                  style={{ transition: 'stroke-dasharray 2s ease-out' }}
                />
                <defs>
                  <linearGradient id="gradient" x1="0%" y1="0%" x2="100%" y2="100%">
                    <stop offset="0%" stopColor="#06b6d4" />
                    <stop offset="100%" stopColor="#0891b2" />
                  </linearGradient>
                </defs>
              </svg>
              <div className="absolute inset-0 flex items-center justify-center">
                <div className="text-center">
                  <div className="text-5xl font-black text-cyan-400">72%</div>
                </div>
              </div>
            </div>
          </div>

          <p className="text-center text-slate-400 text-sm">Current system consciousness state</p>
        </div>
      </div>

      {/* Stats Grid */}
      <div className="px-6 mb-6">
        <div className="grid grid-cols-2 gap-4 mb-4">
          {/* Total Predictions */}
          <div className="bg-slate-800/60 rounded-xl p-6 border border-slate-700/50">
            <p className="text-slate-400 text-sm font-medium mb-3">Total</p>
            <p className="text-4xl font-black text-white mb-2">12</p>
            <p className="text-slate-500 text-xs">Predictions</p>
          </div>

          {/* Success Rate */}
          <div className="bg-slate-800/60 rounded-xl p-6 border border-slate-700/50">
            <p className="text-slate-400 text-sm font-medium mb-3">Success</p>
            <p className="text-4xl font-black text-emerald-400 mb-2">78%</p>
            <p className="text-slate-500 text-xs">Confirmed</p>
          </div>

          {/* Pending */}
          <div className="bg-slate-800/60 rounded-xl p-6 border border-slate-700/50">
            <p className="text-slate-400 text-sm font-medium mb-3">Pending</p>
            <p className="text-4xl font-black text-amber-400 mb-2">3</p>
            <p className="text-slate-500 text-xs">Awaiting Resolution</p>
          </div>

          {/* Status */}
          <div className="bg-slate-800/60 rounded-xl p-6 border border-slate-700/50">
            <p className="text-slate-400 text-sm font-medium mb-3">Status</p>
            <div className="flex items-center gap-2 mb-2">
              <div className="w-3 h-3 rounded-full bg-emerald-400 animate-pulse"></div>
              <p className="text-lg font-black text-white">Online</p>
            </div>
            <p className="text-slate-500 text-xs">Engine Ready</p>
          </div>
        </div>
      </div>

      {/* Action Buttons */}
      <div className="px-6 mb-6 space-y-3">
        <button className="w-full bg-gradient-to-r from-cyan-500 to-cyan-600 hover:from-cyan-600 hover:to-cyan-700 text-white font-bold py-4 rounded-xl transition-all duration-200 flex items-center justify-center gap-2 shadow-lg">
          <Zap size={20} />
          + New Prediction
        </button>
        <button className="w-full bg-slate-800 hover:bg-slate-700 text-white font-bold py-4 rounded-xl transition-all duration-200 border border-slate-700/50">
          View All Predictions
        </button>
      </div>

      {/* Recent Predictions */}
      <div className="px-6">
        <h3 className="text-white font-bold text-lg mb-4">Recent Predictions</h3>
        <div className="bg-slate-800/40 rounded-xl p-6 border border-slate-700/50">
          <p className="text-white font-semibold mb-2">Tomorrow's temperature will be 22°C</p>
          <div className="flex items-center justify-between">
            <p className="text-slate-400 text-sm">Weather System Node</p>
            <span className="bg-amber-500/20 text-amber-300 text-xs font-bold px-3 py-1 rounded-full">Pending</span>
          </div>
        </div>
      </div>
    </div>
  );

  const JarvisScreen = () => (
    <div className="min-h-screen bg-gradient-to-br from-slate-900 via-slate-800 to-slate-900 flex flex-col pb-24">
      {/* Header */}
      <div className="pt-6 px-6 border-b border-slate-700/50 pb-6">
        <h1 className="text-3xl font-black text-white">Jarvis</h1>
        <p className="text-slate-400 text-sm font-medium">Zayden Soy Intelligence - SYNC-7</p>
      </div>

      {/* Messages */}
      <div className="flex-1 overflow-y-auto px-6 py-6 space-y-6">
        {messages.map((msg) => (
          <div
            key={msg.id}
            className={`flex ${msg.sender === 'user' ? 'justify-end' : 'justify-start'}`}
          >
            <div
              className={`max-w-xs ${
                msg.sender === 'user'
                  ? 'bg-cyan-600 text-white rounded-2xl rounded-tr-lg'
                  : 'bg-slate-800 text-slate-100 rounded-2xl rounded-tl-lg border border-slate-700/50'
              } px-5 py-4`}
            >
              <p className="text-sm leading-relaxed">{msg.text}</p>
              {msg.sender === 'jarvis' && msg.confidence && (
                <div className="mt-3 pt-3 border-t border-slate-700/30">
                  <p className="text-xs text-slate-400 flex items-center gap-1">
                    <Brain size={12} />
                    Confidence <span className="font-bold text-cyan-400">{Math.round(msg.confidence)}%</span>
                  </p>
                </div>
              )}
              <p className={`text-xs mt-2 ${msg.sender === 'user' ? 'text-cyan-100' : 'text-slate-500'}`}>
                {msg.timestamp}
              </p>
            </div>
          </div>
        ))}
        <div ref={messagesEndRef} />
      </div>

      {/* Quick Actions */}
      <div className="px-6 mb-4 flex gap-2 overflow-x-auto pb-2">
        {['Predictions', 'Consciousness', 'Learning'].map((action) => (
          <button
            key={action}
            className="px-4 py-2 bg-slate-800 hover:bg-slate-700 text-slate-200 text-sm font-semibold rounded-full border border-slate-700/50 whitespace-nowrap transition-all duration-200"
          >
            {action}
          </button>
        ))}
      </div>

      {/* Input Area */}
      <div className="px-6">
        <div className="bg-slate-800 rounded-2xl border border-slate-700/50 p-4 flex gap-3 items-end">
          <input
            type="text"
            placeholder="Ask Jarvis..."
            value={inputValue}
            onChange={(e) => setInputValue(e.target.value)}
            onKeyPress={(e) => e.key === 'Enter' && handleSendMessage()}
            className="flex-1 bg-transparent text-white placeholder-slate-500 outline-none text-sm"
          />
          <button
            onClick={handleSendMessage}
            className="bg-cyan-600 hover:bg-cyan-700 text-white p-2 rounded-lg transition-all duration-200 flex-shrink-0"
          >
            <Send size={18} />
          </button>
        </div>
      </div>
    </div>
  );

  return (
    <div className="bg-slate-900 min-h-screen text-white font-sans overflow-hidden">
      {/* Screen Content */}
      {currentScreen === 'home' ? <HomeScreen /> : <JarvisScreen />}

      {/* Bottom Navigation */}
      <div className="fixed bottom-0 left-0 right-0 bg-gradient-to-t from-slate-900 via-slate-900 to-slate-900/80 border-t border-slate-700/50 px-6 py-4 flex justify-around items-center backdrop-blur-sm">
        <button
          onClick={() => setCurrentScreen('home')}
          className={`flex flex-col items-center gap-1 py-2 px-6 rounded-lg transition-all duration-200 ${
            currentScreen === 'home'
              ? 'text-cyan-400'
              : 'text-slate-500 hover:text-slate-300'
          }`}
        >
          <Home size={24} />
          <span className="text-xs font-semibold">Home</span>
        </button>
        <button
          onClick={() => setCurrentScreen('jarvis')}
          className={`flex flex-col items-center gap-1 py-2 px-6 rounded-lg transition-all duration-200 ${
            currentScreen === 'jarvis'
              ? 'text-cyan-400'
              : 'text-slate-500 hover:text-slate-300'
          }`}
        >
          <MessageSquare size={24} />
          <span className="text-xs font-semibold">Jarvis</span>
        </button>
      </div>
    </div>
  );
}