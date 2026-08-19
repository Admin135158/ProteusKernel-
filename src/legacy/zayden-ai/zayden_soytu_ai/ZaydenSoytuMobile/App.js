import React, { useState, useEffect, useRef } from 'react';
import {
  SafeAreaView,
  StyleSheet,
  Text,
  TextInput,
  TouchableOpacity,
  View,
  FlatList,
  ScrollView,
  ActivityIndicator,
  StatusBar,
  Dimensions
} from 'react-native';
import io from 'socket.io-client';
import axios from 'axios';
import Icon from 'react-native-vector-icons/MaterialCommunityIcons';

const { width } = Dimensions.get('window');

// API URL - CHANGE THIS TO YOUR MACHINE'S IP
const API_URL = 'http://192.168.18.39:5000'; // Replace with your actual IP

const App = () => {
  const [socket, setSocket] = useState(null);
  const [messages, setMessages] = useState([]);
  const [inputText, setInputText] = useState('');
  const [loading, setLoading] = useState(false);
  const [stats, setStats] = useState({
    uptime: 0,
    nodes: 0,
    genes: 0,
    activeGenes: 0
  });
  const [genes, setGenes] = useState([]);
  const [nodes, setNodes] = useState([]);
  const [activeTab, setActiveTab] = useState('chat');
  const flatListRef = useRef(null);

  useEffect(() => {
    // Connect to WebSocket
    const newSocket = io(API_URL);
    setSocket(newSocket);

    newSocket.on('connect', () => {
      addMessage('System', 'Connected to Zayden Soytu AI', '#00ff88');
    });

    newSocket.on('new_response', (data) => {
      addMessage('ZS AI', data.response, '#00ff88', `Confidence: ${data.confidence.toFixed(1)}%`);
      setLoading(false);
    });

    newSocket.on('evolution', (data) => {
      addMessage('System', `Evolution Generation ${data.generation}`, '#ffaa00');
      fetchGenes();
    });

    // Initial data fetch
    fetchStats();
    fetchGenes();
    fetchNodes();

    // Poll for updates
    const interval = setInterval(() => {
      fetchStats();
      fetchGenes();
      fetchNodes();
    }, 5000);

    return () => {
      clearInterval(interval);
      newSocket.disconnect();
    };
  }, []);

  const addMessage = (sender, text, color = '#00ff88', meta = '') => {
    setMessages(prev => [...prev, {
      id: Date.now().toString(),
      sender,
      text,
      color,
      meta,
      timestamp: new Date().toLocaleTimeString()
    }]);
    
    // Scroll to bottom
    setTimeout(() => {
      flatListRef.current?.scrollToEnd({ animated: true });
    }, 100);
  };

  const fetchStats = async () => {
    try {
      const response = await axios.get(`${API_URL}/api/status`);
      setStats(response.data);
    } catch (error) {
      console.error('Failed to fetch stats:', error);
    }
  };

  const fetchGenes = async () => {
    try {
      const response = await axios.get(`${API_URL}/api/genes`);
      setGenes(response.data.genes || []);
    } catch (error) {
      console.error('Failed to fetch genes:', error);
    }
  };

  const fetchNodes = async () => {
    try {
      const response = await axios.get(`${API_URL}/api/nodes`);
      setNodes(response.data.nodes || []);
    } catch (error) {
      console.error('Failed to fetch nodes:', error);
    }
  };

  const sendPrompt = async () => {
    if (!inputText.trim()) return;

    addMessage('You', inputText, '#ffaa00');
    setLoading(true);

    try {
      await axios.post(`${API_URL}/api/query`, {
        prompt: inputText
      });
      setInputText('');
    } catch (error) {
      addMessage('System', `Error: ${error.message}`, '#ff5555');
      setLoading(false);
    }
  };

  const formatUptime = (seconds) => {
    const hours = Math.floor(seconds / 3600);
    const minutes = Math.floor((seconds % 3600) / 60);
    const secs = seconds % 60;
    
    if (hours > 0) return `${hours}h ${minutes}m`;
    if (minutes > 0) return `${minutes}m ${secs}s`;
    return `${secs}s`;
  };

  const renderChat = () => (
    <View style={styles.chatContainer}>
      <FlatList
        ref={flatListRef}
        data={messages}
        keyExtractor={item => item.id}
        style={styles.messageList}
        renderItem={({ item }) => (
          <View style={styles.messageItem}>
            <Text style={[styles.messageSender, { color: item.color }]}>
              {item.sender}
            </Text>
            <Text style={styles.messageText}>{item.text}</Text>
            {item.meta ? (
              <Text style={styles.messageMeta}>{item.meta}</Text>
            ) : null}
            <Text style={styles.messageTime}>{item.timestamp}</Text>
          </View>
        )}
      />
      
      <View style={styles.inputContainer}>
        <TextInput
          style={styles.input}
          value={inputText}
          onChangeText={setInputText}
          placeholder="Enter your prompt..."
          placeholderTextColor="#666"
          multiline
          onSubmitEditing={sendPrompt}
        />
        <TouchableOpacity 
          style={[styles.sendButton, loading && styles.sendButtonDisabled]}
          onPress={sendPrompt}
          disabled={loading}
        >
          {loading ? (
            <ActivityIndicator color="#000" />
          ) : (
            <Icon name="send" size={24} color="#000" />
          )}
        </TouchableOpacity>
      </View>
    </View>
  );

  const renderStats = () => (
    <ScrollView style={styles.tabContent}>
      <View style={styles.statsGrid}>
        <View style={styles.statCard}>
          <Icon name="clock-outline" size={32} color="#00ff88" />
          <Text style={styles.statLabel}>Uptime</Text>
          <Text style={styles.statValue}>{formatUptime(stats.uptime)}</Text>
        </View>
        
        <View style={styles.statCard}>
          <Icon name="server-network" size={32} color="#00ff88" />
          <Text style={styles.statLabel}>Nodes</Text>
          <Text style={styles.statValue}>{stats.nodes || 0}</Text>
        </View>
        
        <View style={styles.statCard}>
          <Icon name="dna" size={32} color="#00ff88" />
          <Text style={styles.statLabel}>Genes</Text>
          <Text style={styles.statValue}>{stats.genes || 0}</Text>
        </View>
        
        <View style={styles.statCard}>
          <Icon name="flash" size={32} color="#00ff88" />
          <Text style={styles.statLabel}>Active</Text>
          <Text style={styles.statValue}>{stats.activeGenes || 0}</Text>
        </View>
      </View>

      <Text style={styles.sectionTitle}>🧬 ACTIVE GENES</Text>
      {genes.map((gene, index) => (
        <View key={index} style={styles.geneItem}>
          <View style={styles.geneHeader}>
            <View style={styles.geneNameContainer}>
              <Text style={styles.geneName}>{gene.name}</Text>
              <Text style={styles.geneSpecies}>[{gene.species}]</Text>
              {gene.active && <Icon name="circle" size={12} color="#00ff88" />}
            </View>
            <Text style={styles.geneStrength}>{gene.strength.toFixed(1)}%</Text>
          </View>
          <View style={styles.strengthBar}>
            <View style={[styles.strengthFill, { width: `${gene.strength}%` }]} />
          </View>
          <View style={styles.geneStats}>
            <Text style={styles.geneStat}>μ:{gene.mutations}</Text>
            <Text style={styles.geneStat}>⚔️:{gene.kills}</Text>
            <Text style={styles.geneStat}>🧬:{gene.children}</Text>
          </View>
        </View>
      ))}

      <Text style={styles.sectionTitle}>🌐 NETWORK NODES</Text>
      {nodes.map((node, index) => (
        <View key={index} style={styles.nodeItem}>
          <View style={styles.nodeHeader}>
            <Text style={styles.nodeId}>Node {node.id}</Text>
            <Text style={styles.nodePlatform}>{node.platform}</Text>
          </View>
          {node.stats?.models && (
            <View style={styles.ensembleStats}>
              {node.stats.models.map((model, idx) => (
                <Text key={idx} style={styles.modelStat}>
                  📊 {model.name}: {model.queries} queries
                </Text>
              ))}
            </View>
          )}
        </View>
      ))}
    </ScrollView>
  );

  return (
    <SafeAreaView style={styles.container}>
      <StatusBar barStyle="light-content" backgroundColor="#0a0a0a" />
      
      <View style={styles.header}>
        <Text style={styles.title}>ZAYDEN SOYTU AI</Text>
        <Text style={styles.subtitle}>SYNC-7 PROTOCOL</Text>
      </View>

      <View style={styles.tabBar}>
        <TouchableOpacity 
          style={[styles.tab, activeTab === 'chat' && styles.activeTab]}
          onPress={() => setActiveTab('chat')}
        >
          <Icon name="chat" size={24} color={activeTab === 'chat' ? '#00ff88' : '#666'} />
          <Text style={[styles.tabText, activeTab === 'chat' && styles.activeTabText]}>Chat</Text>
        </TouchableOpacity>
        
        <TouchableOpacity 
          style={[styles.tab, activeTab === 'stats' && styles.activeTab]}
          onPress={() => setActiveTab('stats')}
        >
          <Icon name="chart-box" size={24} color={activeTab === 'stats' ? '#00ff88' : '#666'} />
          <Text style={[styles.tabText, activeTab === 'stats' && styles.activeTabText]}>Stats</Text>
        </TouchableOpacity>
      </View>

      {activeTab === 'chat' ? renderChat() : renderStats()}
    </SafeAreaView>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#0a0a0a',
  },
  header: {
    padding: 20,
    borderBottomWidth: 1,
    borderBottomColor: '#00ff88',
    alignItems: 'center',
  },
  title: {
    fontSize: 24,
    fontWeight: 'bold',
    color: '#00ff88',
    letterSpacing: 2,
  },
  subtitle: {
    fontSize: 12,
    color: '#666',
    marginTop: 5,
  },
  tabBar: {
    flexDirection: 'row',
    borderBottomWidth: 1,
    borderBottomColor: '#333',
  },
  tab: {
    flex: 1,
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'center',
    padding: 15,
    gap: 10,
  },
  activeTab: {
    borderBottomWidth: 2,
    borderBottomColor: '#00ff88',
  },
  tabText: {
    color: '#666',
    fontSize: 16,
  },
  activeTabText: {
    color: '#00ff88',
  },
  chatContainer: {
    flex: 1,
  },
  messageList: {
    flex: 1,
    padding: 15,
  },
  messageItem: {
    marginBottom: 15,
    padding: 10,
    borderLeftWidth: 3,
    borderLeftColor: '#00ff88',
    backgroundColor: 'rgba(0,255,136,0.02)',
  },
  messageSender: {
    fontWeight: 'bold',
    marginBottom: 5,
  },
  messageText: {
    color: '#fff',
    marginLeft: 10,
  },
  messageMeta: {
    color: '#666',
    fontSize: 12,
    marginTop: 5,
    marginLeft: 10,
  },
  messageTime: {
    color: '#444',
    fontSize: 10,
    marginTop: 5,
    textAlign: 'right',
  },
  inputContainer: {
    flexDirection: 'row',
    padding: 15,
    borderTopWidth: 1,
    borderTopColor: '#333',
    backgroundColor: '#0a0a0a',
  },
  input: {
    flex: 1,
    backgroundColor: '#1a1a1a',
    color: '#00ff88',
    padding: 15,
    borderRadius: 10,
    marginRight: 10,
    maxHeight: 100,
  },
  sendButton: {
    width: 50,
    height: 50,
    backgroundColor: '#00ff88',
    borderRadius: 25,
    justifyContent: 'center',
    alignItems: 'center',
  },
  sendButtonDisabled: {
    opacity: 0.5,
  },
  tabContent: {
    flex: 1,
    padding: 15,
  },
  statsGrid: {
    flexDirection: 'row',
    flexWrap: 'wrap',
    justifyContent: 'space-between',
    marginBottom: 20,
  },
  statCard: {
    width: (width - 50) / 2,
    backgroundColor: 'rgba(0,255,136,0.05)',
    borderWidth: 1,
    borderColor: '#00ff88',
    borderRadius: 10,
    padding: 15,
    alignItems: 'center',
    marginBottom: 10,
  },
  statLabel: {
    color: '#666',
    fontSize: 12,
    marginTop: 5,
  },
  statValue: {
    color: '#00ff88',
    fontSize: 20,
    fontWeight: 'bold',
  },
  sectionTitle: {
    color: '#00ff88',
    fontSize: 18,
    fontWeight: 'bold',
    marginVertical: 15,
  },
  geneItem: {
    backgroundColor: 'rgba(0,255,136,0.05)',
    borderWidth: 1,
    borderColor: '#00ff88',
    borderRadius: 10,
    padding: 15,
    marginBottom: 10,
  },
  geneHeader: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    marginBottom: 10,
  },
  geneNameContainer: {
    flexDirection: 'row',
    alignItems: 'center',
    gap: 10,
  },
  geneName: {
    color: '#00ff88',
    fontWeight: 'bold',
    fontSize: 16,
  },
  geneSpecies: {
    color: '#ffaa00',
    fontSize: 12,
  },
  geneStrength: {
    color: '#00ff88',
    fontWeight: 'bold',
    fontSize: 16,
  },
  strengthBar: {
    height: 5,
    backgroundColor: 'rgba(0,255,136,0.2)',
    borderRadius: 3,
    overflow: 'hidden',
    marginBottom: 10,
  },
  strengthFill: {
    height: '100%',
    backgroundColor: '#00ff88',
  },
  geneStats: {
    flexDirection: 'row',
    gap: 20,
  },
  geneStat: {
    color: '#666',
    fontSize: 12,
  },
  nodeItem: {
    backgroundColor: 'rgba(0,255,136,0.05)',
    borderWidth: 1,
    borderColor: '#00ff88',
    borderRadius: 10,
    padding: 15,
    marginBottom: 10,
  },
  nodeHeader: {
    flexDirection: 'row',
    alignItems: 'center',
    gap: 10,
  },
  nodeId: {
    color: '#00ff88',
    fontWeight: 'bold',
    fontSize: 14,
  },
  nodePlatform: {
    color: '#fff',
    fontSize: 14,
  },
  ensembleStats: {
    marginTop: 10,
    paddingTop: 10,
    borderTopWidth: 1,
    borderTopColor: '#333',
  },
  modelStat: {
    color: '#666',
    fontSize: 12,
    marginVertical: 2,
  },
});

export default App;
