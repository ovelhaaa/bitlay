# Delta Delay CVSD (Bitlay) v1.1

Bitlay (Delta Delay CVSD) é um delay digital vintage de 1-bit construído com React, TypeScript e Vite. O plugin emula a degradação e o caráter único dos clássicos atrasos digitais iniciais (como os baseados na família de chips Lexicon Delta T e delays em pedais antigos de 1-bit) usando o algoritmo de Modulação Delta Continuamente Variável (CVSD - Continuously Variable Slope Delta modulation).

## Changelog

- **v1.1**: Correção do comportamento sonoro do DSP. Anteriormente, quando o `feedback` estava zerado, a saída direta não aplicava o atraso (entregava o sinal codificado instantaneamente). Agora, a saída direta também utiliza o buffer de delay (`finalDelayedSample`), garantindo que o tempo de delay afete o som mesmo sem feedback.

## Arquitetura Geral

O projeto é dividido em duas frentes que se comunicam através da Web Audio API:

1. **React UI**: Toda a interface de usuário (parâmetros, tap tempo, botões e controles) vive no ecossistema React. Os estados complexos gerenciam o painel do delay.
2. **AudioWorklet**: O processamento de sinal em tempo real (DSP) ocorre isoladamente num AudioWorklet `CVSDDelayProcessor`.
   A comunicação entre a UI (React) e o Worklet acontece exclusivamente através de **AudioParams** conectados em tempo real. Sempre que você move um knob na interface React, ele muda o respectivo AudioParam a-rate ou k-rate, garantindo mudanças instantâneas e suaves, sem travamentos (glitches) no áudio.

## Como o algoritmo CVSD funciona

Em contraste aos delays modernos PCM (onde o áudio é armazenado em blocos de 16 ou 24 bits por amostra), a **Modulação Delta** rastreia o sinal registrando apenas a diferença (delta) entre a amostra atual e a anterior, gravando essa variação como um fluxo de `1`s (sinal subindo) e `0`s (sinal descendo).

O diferencial do **CVSD** (Continuously Variable Slope Delta) é que o tamanho do "passo" adaptativamente aumenta quando há variações rápidas e grandes de sinal, e diminui em partes calmas. Se ocorrem muitos "1s" (sinal forte subindo) ou muitos "0s" (sinal forte descendo), um envelope silábico expande dinamicamente a taxa de variação. Essa dinâmica resulta numa recriação sonora nostálgica cheia de harmônicos ímpares, limitação dinâmica agressiva e "aliasing" ruidoso controlado pelo relógio (clock).

## Principais Parâmetros do Worklet

Os parâmetros do `CVSDDelayProcessor` são expostos como AudioParams:

- `delayTime`, `feedback`, `mix`, `bypass`: Controles padrão de delay.
- `stepSize`: Define a taxa base de decodificação do áudio 1-bit.
- `clockJitter`: Introduz erros e falhas intermitentes no clock de amostragem.
- `integratorLag` e `integratorLeak`: Controles do filtro integrador responsável por reconstruir a onda sonora.
- `reconstructionCutoff`: Filtro passa-baixa anti-aliasing aplicado antes do output.
- `inputDrive`: Ganho de entrada antes de entrar na malha de conversão 1-bit.
- `dynamicResponse`: Define o quão rápido o envelope do CVSD se adapta ao áudio (slope/syllabic time).
- `wobbleDepth` e `wobbleRate`: Osciladores de baixa frequência (LFOs) para a velocidade do relógio ou "tape flutter".
- `feedbackTone`: Controle de tonalidade (Low pass) da malha de feedback.
- `stereoSpread`: Cria um campo pseudo-estéreo defasando o clock L/R.
- `circuitType`: Altera o funcionamento de discreto (linear) para companded (dinâmico).
- **Multi-tap Params** (`numTaps`, `tap1_mult`, `tap2_mix`, etc.): Permitem que a linha de delay leia o áudio em múltiplas instâncias diferentes criando ritmos.
- **Reverse Mode** (`reverseMode`, `reverseChunkSize`, etc.): Modo em que a reprodução do buffer ocorre de trás pra frente por pedaços.

## Setup Local

Para rodar este repositório localmente:

1. Instale as dependências:

   ```bash
   npm install
   ```

2. Inicie o servidor de desenvolvimento:

   ```bash
   npm run dev
   ```

3. Abra `http://localhost:3000` no seu navegador. O Vite atualizará a tela em tempo real graças ao Hot Module Replacement (HMR).

Para construir para produção:

```bash
npm run build
```
