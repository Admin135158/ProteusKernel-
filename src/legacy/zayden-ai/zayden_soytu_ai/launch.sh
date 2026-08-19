#!/bin/bash

# ZAYDEN SOYTU AI - LAUNCH SEQUENCE

GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${BLUE}╔══════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║     🚀 ZAYDEN SOYTU AI - LAUNCH SEQUENCE                ║${NC}"
echo -e "${BLUE}╚══════════════════════════════════════════════════════════╝${NC}"

# Check if we're in the right directory
if [ ! -d "venv" ]; then
    echo -e "${RED}❌ Not in Zayden Soytu directory or venv missing${NC}"
    echo "   Run this from ~/zayden_soytu_ai"
    exit 1
fi

# Activate virtual environment
echo -e "${GREEN}✅ Activating virtual environment${NC}"
source venv/bin/activate

# Check for .env file
if [ ! -f ".env" ]; then
    echo -e "${YELLOW}⚠️  .env file not found. Creating template...${NC}"
    cat > .env << 'EOF'
# ZAYDEN SOYTU AI - API KEYS
OPENAI_KEY=your_openai_key_here
ANTHROPIC_KEY=your_anthropic_key_here
GEMINI_KEY=your_gemini_key_here
COHERE_KEY=your_cohere_key_here
DEEPSEEK_KEY=your_deepseek_key_here
MISTRAL_KEY=your_mistral_key_here
PERPLEXITY_KEY=your_perplexity_key_here
GROK_KEY=your_grok_key_here

# Architect Constants
O=9
PHI=1.618033988749895
N=3
EOF
    echo -e "${YELLOW}⚠️  Edit .env with your API keys before using real APIs${NC}"
fi

# Check for genes directory
if [ ! -d "genes" ] || [ -z "$(ls -A genes 2>/dev/null)" ]; then
    echo -e "${YELLOW}⚠️  No genes found. Will create seed genes.${NC}"
fi

# Launch Zayden Soytu AI
echo -e "${GREEN}🔥 Launching ZAYDEN SOYTU AI...${NC}"
echo ""

python zayden_soytu.py

# Deactivate on exit
deactivate
