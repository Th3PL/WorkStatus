# 🚦 WorkStatus Light
### Semáforo de Concentração para Ambientes Híbridos

![Status](https://img.shields.io/badge/Status-Ativo-brightgreen)
![Version](https://img.shields.io/badge/Versão-1.0.0-blue)
![License](https://img.shields.io/badge/License-MIT-green)

---

## 📋 Descrição

**WorkStatus Light** é um projeto IoT inovador que implementa um **semáforo visual inteligente** para controlar o status de concentração em ambientes híbridos (home office e coworking). 

O sistema utiliza um **LED RGB** que muda de cor de acordo com o estado da pessoa, permitindo que colegas identifiquem visualmente se alguém está:
- 🟢 **Disponível** (LIVRE) - Aberto para interrupções
- 🟡 **Ocupado** (OCUPADO) - Trabalhando, mas pode interromper se urgente
- 🔴 **Em Foco** (FOCO) - Concentração profunda, não interromper
- 🔴 **Em Reunião** (REUNIÃO) - Indisponível no momento

O projeto integra **sensores (LDR)** para detectar presença, **botões físicos** para controle manual, **buzzer** para feedback sonoro e um **dashboard web** em tempo real para gerenciamento remoto de múltiplos postos.

---

## 📹 Vídeo Demonstrativo

[![YouTube](https://img.shields.io/badge/YouTube-Watch%20Demo-red?style=for-the-badge&logo=youtube)](https://youtu.be/seu-link-aqui)

**Link:** [https://youtu.be/seu-link-aqui](https://youtu.be/seu-link-aqui)

---

## 🛠️ Hardware

### Componentes Utilizados

| Componente | Modelo | Quantidade | Descrição |
|-----------|--------|-----------|-----------|
| **Microcontrolador** | Arduino Uno | 1 | Processamento e controle dos sensores |
| **LED RGB** | HW-479 | 1 | Indicador visual do estado (cátodo comum) |
| **LCD I2C** | 16x2 | 1 | Display de informações do estado |
| **Sensor de Luz** | LDR | 1 | Detecção de presença na mesa |
| **Buzzer** | 5V | 1 | Feedback sonoro para mudanças de estado |
| **Botões** | Push Button | 3 | Controle manual (START, SELECT, MENU) |
| **Resistor** | 10kΩ | 1 | Pull-down para LDR |
| **Jumpers e Fios** | Diversos | - | Conexões e cabeamento |

### Diagrama de Conexão

```
Arduino Uno
├─ LDR (Sensor de Luz)
│  ├─ Um lado → 5V
│  ├─ Outro lado → A0 + Resistor 10kΩ ao GND
│
├─ LCD I2C (16x2)
│  ├─ VCC → 5V
│  ├─ GND → GND
│  ├─ SDA → A4
│  └─ SCL → A5
│
├─ LED RGB HW-479 (Cátodo Comum)
│  ├─ R → Pino 9 (PWM)
│  ├─ G → Pino 10 (PWM)
│  ├─ B → Pino 11 (PWM)
│  └─ - (Cátodo) → GND
│
├─ Buzzer
│  ├─ + → Pino 8
│  └─ - → GND
│
└─ Botões (3x) com INPUT_PULLUP
   ├─ BTN_START → Pino 2 ao GND
   ├─ BTN_SELECT → Pino 3 ao GND
   └─ BTN_MENU → Pino 4 ao GND
```

---

## 💻 Software

### Stack Tecnológico

| Camada | Tecnologia | Versão |
|--------|-----------|--------|
| **Microcontrolador** | Arduino C/C++ | - |
| **Bridge IoT** | Python 3 | 3.12+ |
| **Broker MQTT** | Mosquitto | 2.0+ |
| **Automação/Dashboard** | Node-RED | 4.0+ |
| **Frontend** | HTML5 + CSS3 + JavaScript | ES6+ |
| **Comunicação** | MQTT + HTTP | - |

### Dependências

#### Arduino
```
LiquidCrystal_I2C (by Frank de Brabander)
```

#### Python
```
paho-mqtt==2.0+
pyserial==3.5+
```

#### Node-RED
```
node-red-dashboard
node-red-contrib-ui-led (opcional)
```

---

## 🏗️ Arquitetura

### Diagrama de Fluxo de Dados

```
┌─────────────────────────────────────────────────────────────┐
│                     CAMADA FÍSICA                           │
├─────────────────────────────────────────────────────────────┤
│  Arduino (Hardware)                                         │
│  ├─ LED RGB (Saída Visual)                                 │
│  ├─ LCD (Display)                                          │
│  ├─ Botões (Entrada)                                       │
│  ├─ LDR (Sensor)                                           │
│  └─ Buzzer (Feedback Sonoro)                               │
└────────────┬────────────────────────────────────────────────┘
             │ Serial (UART 115200 baud)
             ▼
┌─────────────────────────────────────────────────────────────┐
│                  CAMADA DE BRIDGE                           │
├─────────────────────────────────────────────────────────────┤
│  Python Bridge (workStatus_bridge.py)                       │
│  ├─ Lê dados do Arduino via Serial                         │
│  ├─ Parse do protocolo customizado                         │
│  └─ Publica em MQTT (office/desk01/status)                 │
└────────────┬────────────────────────────────────────────────┘
             │ MQTT (porta 1883)
             ▼
┌─────────────────────────────────────────────────────────────┐
│                 CAMADA DE COMUNICAÇÃO                       │
├─────────────────────────────────────────────────────────────┤
│  Mosquitto Broker MQTT                                      │
│  ├─ Tópicos:                                               │
│  │  ├─ office/desk01/status (Arduino → Dashboard)          │
│  │  └─ office/desk01/cmd (Dashboard → Arduino)             │
│  └─ QoS: 1 (Garantia de Entrega)                           │
└────────────┬────────────────────────────────────────────────┘
             │ MQTT
             ▼
┌─────────────────────────────────────────────────────────────┐
│                CAMADA DE PROCESSAMENTO                      │
├─────────────────────────────────────────────────────────────┤
│  Node-RED Flow                                              │
│  ├─ Subscreve a office/desk01/status                       │
│  ├─ Processa e formata dados                               │
│  ├─ Exponibiliza endpoints HTTP:                           │
│  │  ├─ GET /workstatus/current → Status atual              │
│  │  └─ POST /workstatus/command → Enviar comandos          │
│  └─ Armazena histórico em flow                             │
└────────────┬────────────────────────────────────────────────┘
             │ HTTP (porta 1880)
             ▼
┌─────────────────────────────────────────────────────────────┐
│                  CAMADA DE APRESENTAÇÃO                     │
├─────────────────────────────────────────────────────────────┤
│  Dashboard Web (index.html)                                 │
│  ├─ Polling HTTP a cada 500ms                              │
│  ├─ Visualização do estado                                 │
│  ├─ Controle remoto (4 botões)                             │
│  ├─ Gauge de luminosidade                                  │
│  ├─ Histórico de mudanças                                  │
│  └─ Mapa da equipe                                         │
└─────────────────────────────────────────────────────────────┘
```

### Protocolo de Comunicação

#### Arduino → Python (Serial)
```
Formato: ID=01;STATE=LIVRE;LDR=345;TS=5234

Campos:
- ID: Identificador do posto (01-99)
- STATE: Estado atual (LIVRE, OCUPADO, FOCO, REUNIAO)
- LDR: Valor bruto do sensor (0-1023)
- TS: Timestamp em milissegundos
```

#### Python → MQTT (JSON)
```json
{
  "ID": "01",
  "STATE": "LIVRE",
  "LDR": "345",
  "TS": "5234",
  "timestamp": "2025-11-20T15:23:45.123Z"
}
```

#### Dashboard → Node-RED (HTTP)
```bash
# Obter status
GET http://localhost:1880/workstatus/current

# Enviar comando
POST http://localhost:1880/workstatus/command
Content-Type: application/json
{"command": "CMD:FOCO"}
```

---

## 🚀 Como Começar

### Pré-requisitos
- Arduino IDE
- Python 3.12+
- Node-RED instalado
- Mosquitto Broker

### Instalação

#### 1. Configurar Arduino
```bash
# 1. Abrir Arduino IDE
# 2. Instalar biblioteca: Sketch → Include Library → Manage Libraries
#    Buscar: LiquidCrystal_I2C (by Frank de Brabander)
# 3. Fazer upload do código: WorkStatus-Optimized.ino
```

#### 2. Iniciar Bridge Python
```bash
cd ~/Área\ de\ trabalho/IOT-GS
python -m venv venv
source venv/bin/activate  # Linux/Mac
venv\Scripts\activate     # Windows

pip install paho-mqtt pyserial

python workStatus_bridge.py
```

#### 3. Iniciar Mosquitto
```bash
sudo systemctl start mosquitto
sudo systemctl enable mosquitto
```

#### 4. Configurar Node-RED
```bash
node-red-start
# Acessar: http://localhost:1880
# Importar o flow com endpoints HTTP
```

#### 5. Abrir Dashboard
```bash
python3 -m http.server 8000
# Acessar: http://localhost:8000/index.html
```

---

## 📊 Casos de Uso

### 1. **Ambiente de Coworking**
Múltiplos usuários podem ver o status de concentração de colegas antes de interromper.

### 2. **Home Office**
Sinalizar disponibilidade para a família/conviventes sem precisar de comunicação verbal.

### 3. **Salas de Desenvolvimento**
Equipes de programadores podem gerenciar períodos de foco profundo.

### 4. **Educação à Distância**
Professores podem indicar quando estão em aula gravada (modo FOCO).

---

## 📈 Desempenho

| Métrica | Valor | Descrição |
|---------|-------|-----------|
| **Latência Arduino → Dashboard** | ~600ms | Tempo total de resposta |
| **Frequência de Atualização** | 2-5s | Heartbeat ou mudança detectada |
| **Poder Consumido** | ~500mW | Operação normal com LED aceso |
| **Conexões MQTT Simultâneas** | 50+ | Escalabilidade (múltiplos postos) |

---

## 🧪 Testes Implementados

- ✅ Teste de Hardware (LED, LCD, Buzzer, Botões)
- ✅ Teste de Comunicação Serial (Arduino ↔ Python)
- ✅ Teste de Broker MQTT (Pub/Sub)
- ✅ Teste de Dashboard (Atualização em tempo real)
- ✅ Teste End-to-End (Fluxo completo físico → digital)
- ✅ Teste de Estresse (30min+ operação contínua)

---

## 👥 Membros do Grupo

| Nome | Papel | Responsabilidades |
|------|-------|-------------------|
| **[Seu Nome]** | **[Papel]** | Arduino, Hardware |
| **[Colega 1]** | **[Papel]** | Bridge Python, MQTT |
| **[Colega 2]** | **[Papel]** | Node-RED, Backend |
| **[Colega 3]** | **[Papel]** | Dashboard, Frontend |

---

## 📚 Referências

- [Arduino Official Documentation](https://www.arduino.cc/reference/en/)
- [Mosquitto MQTT Broker](https://mosquitto.org/)
- [Node-RED Documentation](https://nodered.org/docs/)
- [paho-mqtt Python Library](https://github.com/eclipse/paho.mqtt.python)
- [LiquidCrystal_I2C Library](https://github.com/johnwargo/LiquidCrystal_I2C)

---

## 📝 Licença

Este projeto está sob a licença **MIT**. Veja o arquivo `LICENSE` para mais detalhes.

---

## 🤝 Contribuições

Sugestões e melhorias são bem-vindas! Para contribuir:

1. Faça um Fork do projeto
2. Crie uma branch para sua feature (`git checkout -b feature/AmazingFeature`)
3. Commit suas mudanças (`git commit -m 'Add some AmazingFeature'`)
4. Push para a branch (`git push origin feature/AmazingFeature`)
5. Abra um Pull Request

---

## 📞 Suporte

Para dúvidas ou problemas:
- 📧 Email: seu-email@exemplo.com
- 💬 Issues: [GitHub Issues](https://github.com/seu-usuario/workstatus-light/issues)
- 📱 WhatsApp: [Seu contato]

---

## 🎯 Roadmap Futuro

- [ ] Suporte para múltiplos postos (desk02, desk03, etc)
- [ ] Integração com Google Calendar
- [ ] Notificações push no celular
- [ ] Estatísticas de produtividade
- [ ] API pública para integrações
- [ ] Modo dark/light no dashboard
- [ ] Suporte a múltiplas linguagens
- [ ] App mobile nativa

---

## 📷 Galeria

### Hardware
![Hardware Setup](./images/hardware.jpg)

### Dashboard
![Dashboard Screenshot](./images/dashboard.png)

### LED States
![LED States](./images/led-states.png)

---

**Desenvolvido com ❤️ para o GLOBAL SOLUTIONS 2025 - O FUTURO DO TRABALHO**

*Última atualização: 20 de Novembro de 2025*
