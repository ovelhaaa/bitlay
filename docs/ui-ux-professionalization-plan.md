# Bitlay UI/UX Professionalization Plan

## Objetivo

Transformar o Bitlay de uma interface tecnica de laboratorio em um instrumento musical profissional: claro, desejavel, rapido de operar e coerente com a personalidade sonora do plugin.

A referencia aspiracional nao deve ser copiar Universal Audio visualmente, mas absorver os seus principios: hierarquia forte, poucos controles principais, acabamento material convincente, feedback visual util, presets confiaveis e uma experiencia que comunica som antes de comunicar implementacao.

## Diagnostico Atual

### 1. Posicionamento visual ainda indefinido

A UI nativa mistura neon, cyberpunk, painel de debug e rack tecnico. O resultado chama atencao, mas nao cria uma promessa clara de produto. Plugins consolidados geralmente escolhem uma metafora dominante: rack, pedal, canal de console, fita, modulo vintage ou ferramenta moderna. O Bitlay tem um conceito excelente, delay digital 1-bit/CVSD, mas a UI ainda nao transforma esse conceito em objeto.

Evidencias:

- Paleta com ciano, laranja, roxo e vermelho competindo como acentos principais.
- Knobs com glow forte e contraste alto, mais proximos de instrumento sci-fi do que de hardware premium.
- Osciloscopio ocupa posicao nobre, mas parece diagnostico interno, nao medidor musical.
- Nome "WebStyleLookAndFeel" denuncia que a UI nativa herdou linguagem do prototipo web.

### 2. Hierarquia de uso confusa

O usuario abre o plugin e encontra muitos parametros com peso visual parecido. Delay Time, Feedback e Mix deveriam ser a espinha dorsal. Texture, Movement, Tone/Character e Sync deveriam formar a segunda camada. CVSD internals, envelope, step size e integrator deveriam ser acesso avancado, nao competir com o fluxo principal.

Evidencias:

- A janela inicia em `1000x720`, com header, visualizador, abas e muitos grupos.
- O primeiro tab ainda usa varios grupos, macros, sync, freeze e mix no mesmo plano.
- As abas usam nomes numerados e tecnicos: "MACROS & CORE", "MULTI-TAP ENGINE", "ADVANCED CVSD", "REVERSE & MOD".
- O tab de circuito expoe muitos parametros internos, o que e bom para sound design, mas intimidador como experiencia padrao.

### 3. Controles nao expressam escala musical

O Bitlay e um delay musical, mas alguns controles sao tratados como parametro bruto. Em plugins profissionais, o usuario entende rapidamente o gesto: tempo, repeticoes, cor, instabilidade, largura, degrade, reverse, freeze. O valor tecnico fica disponivel, mas nao domina a leitura.

Problemas principais:

- `wobbleSync` e slider rotativo, mas conceitualmente e modo ou quantidade de sincronismo.
- `stepSize`, `integratorLag`, `integratorLeak`, `syllabicTime` e `dynamicResponse` precisam de nomes de superficie mais musicais.
- `internalBpm` aparece como slider horizontal, mas falta uma area ritmica com BPM, subdivisao, sync host e tap.
- O usuario nao ve claramente quando o delay esta em modo synced versus free time.

### 4. Presets existem, mas nao viraram experiencia

Os presets nativos sao criados automaticamente e tem bons nomes. Porem a UI nativa oferece apenas combo + Save + Save As placeholder. Isso deixa a experiencia aquem de plugins comerciais, onde presets ajudam a descobrir o instrumento.

Problemas:

- Sem categorias ou tags sonoras.
- Sem favoritos.
- Sem estado de preset modificado.
- `Save As...` ainda e placeholder.
- Presets nao comunicam intencao: slapback, space, dirty, rhythmic, reverse, glitch.

### 5. Layout muito rigido

O `resized()` usa coordenadas absolutas. Isso acelera prototipo, mas limita acabamento profissional:

- Dificulta escalonamento 100%, 125%, 150%.
- Dificulta revisao fina de spacing.
- Aumenta risco de texto cortado em hosts com fontes/dpi diferentes.
- Torna mais caro criar variantes compacta/expandida.

### 6. Falta uma camada de design system nativa

Ha um `LookAndFeel`, mas ele ainda mistura tokens, renderizacao e semantica. Uma UI profissional precisa de primitives consistentes:

- paleta semantica;
- tipografia;
- knobs grandes, medios e pequenos;
- botoes de estado;
- medidores;
- labels com unidade;
- seco/molhado, tempo, ritmo e macros com formatos previsiveis;
- areas de painel com profundidade e materialidade.

## Direcao Criativa Recomendada

### Conceito

"Primitive Digital Delay, made playable."

O Bitlay deve parecer um rack digital raro dos anos 80 reinterpretado com acabamento contemporaneo: faceplate escuro, detalhes laranja queimado, medidores discretos, knobs solidos e uma sensacao de instrumento de estudio.

### Paleta

Recomendada:

- fundo: preto quente/carvao;
- faceplate: grafite ou metal escuro;
- acento principal: laranja queimado Bitlay;
- acento funcional: verde/ambar para atividade e sync;
- perigo/estado destrutivo: vermelho apenas para bypass/reverse intenso/clipping;
- ciano: remover como cor principal ou reservar para visualizador tecnico.

### Tipografia

Recomendada:

- sans condensada/industrial para labels e secao;
- numerais tabulares para valores;
- menos texto em caixa alta longa;
- labels curtos, com unidade sempre previsivel.

### Metafora

Evitar painel "app dashboard". Priorizar faceplate de audio:

- topo: marca, preset, A/B, undo/redo, bypass;
- centro: controles principais grandes;
- base ou lateral: modulacao, cor, ritmo, taps;
- painel avancado recolhivel: CVSD Lab.

## Arquitetura de Informacao Proposta

### Camada 1: Performance

Controles sempre visiveis:

- Time
- Feedback
- Mix
- Character
- Degrade
- Movement
- Tone
- Width
- Sync/Free
- Freeze
- Reverse
- Bypass

### Camada 2: Rhythm

Controles ritmicos:

- Host Sync
- BPM quando free/internal
- Main subdivision
- Tap tempo
- Multi-tap count
- Tap pattern editor
- Tap decay/spread

### Camada 3: Tone Engine

Controles de carater:

- Circuit: Discrete / Companded
- Step behavior, em nome musical
- Clock instability
- Reconstruction filter
- Drive
- Envelope response

### Camada 4: CVSD Lab

Parametros tecnicos avancados:

- stepSize
- minStepSize
- maxStepSize
- integratorLag
- integratorLeak
- dynamicResponse
- syllabicTime
- envAttack
- envRelease

Esta camada deve ser recolhivel, com linguagem de laboratorio deliberada, para usuarios avancados.

## Plano de Implementacao

### Fase 1: Fundacao visual e hierarquia

Objetivo: fazer o plugin parecer um produto antes de mexer em funcionalidade profunda.

Tarefas:

- Renomear `WebStyleLookAndFeel` para `BitlayLookAndFeel`.
- Criar tokens de design em C++: cores, spacing, raios, fontes, tamanhos de knob.
- Reduzir a paleta neon para uma paleta premium com acento laranja.
- Criar componentes de UI reutilizaveis: `BitlayKnob`, `BitlayToggle`, `BitlayCombo`, `BitlaySection`.
- Redesenhar o header com marca, preset browser, A/B, undo/redo, bypass.
- Substituir abas numeradas por nomes simples: Main, Rhythm, Engine, Lab.

Resultado esperado:

- A UI passa a ter identidade consistente.
- O primeiro olhar comunica "instrumento de delay", nao "prototipo tecnico".

### Fase 2: Main view tocavel

Objetivo: tornar o fluxo principal rapido e musical.

Tarefas:

- Reorganizar o primeiro painel em tres blocos: Delay, Color, Motion.
- Tornar Time, Feedback e Mix knobs maiores.
- Transformar Degrade e Wobble em macros com escala visual mais expressiva.
- Adicionar readouts claros: ms ou divisao ritmica, porcentagem, Hz.
- Mostrar estado de sync junto ao Time, nao distante dele.
- Criar estados visuais fortes para Freeze, Reverse e Bypass.
- Reduzir o osciloscopio ou converte-lo em medidor/monitor opcional.

Resultado esperado:

- O usuario consegue chegar a um som util em menos de 10 segundos.
- O fluxo principal funciona sem abrir abas avancadas.

### Fase 3: Preset browser profissional

Objetivo: transformar presets em descoberta sonora.

Tarefas:

- Implementar `Save As...`.
- Mostrar estado "edited" quando parametros divergem do preset carregado.
- Adicionar categorias: Clean, Vintage, Dirty, Rhythmic, Reverse, Experimental.
- Adicionar favoritos.
- Permitir next/previous preset no header.
- Criar descricoes curtas dos presets.
- Padronizar nomes nativos em ingles ou portugues, evitando mistura casual.

Resultado esperado:

- Presets deixam de ser apenas arquivos e viram parte da experiencia comercial.

### Fase 4: Rhythm e taps como sequencer visual

Objetivo: fazer multi-tap parecer padrao ritmico, nao tabela de parametros.

Tarefas:

- Criar editor visual de 4 taps com posicao no tempo e nivel.
- Exibir cada tap como coluna/ponto: time/subdivision + level.
- Desabilitar ou diminuir taps inativos conforme `numTaps`.
- Separar `tap_mult` manual de `tap_subdiv` synced.
- Mostrar feedback de host sync/BPM.

Resultado esperado:

- O usuario entende o padrao de eco olhando, antes de ler labels.

### Fase 5: Engine/Lab avancado sem intimidar

Objetivo: preservar profundidade tecnica sem prejudicar o usuario musical.

Tarefas:

- Renomear parametros de superficie:
  - Character: Drive
  - Reconstruction Cutoff: Tone
  - Clock Jitter: Instability
  - Integrator Lag: Slew
  - Integrator Leak: Memory
  - Dynamic Response: Tracking
  - Syllabic Time: Adapt Time
- Criar tooltips curtos e musicais.
- Manter nomes tecnicos em sublabel ou tooltip.
- Agrupar controles por causa sonora: Dirt, Bandwidth, Instability, Compander.

Resultado esperado:

- Sound designers continuam tendo controle fino.
- Musicos nao precisam decifrar CVSD para usar o plugin.

### Fase 6: Acabamento de produto

Objetivo: elevar confiabilidade percebida.

Tarefas:

- Adicionar resize/scaling persistente.
- Implementar double-click reset nos knobs.
- Implementar valores digitaveis nos controles principais.
- Adicionar indicadores de automacao/hover/focus.
- Verificar contraste e legibilidade em DPI alto.
- Criar screenshots e checklist visual por host.
- Testar em Reaper, Ableton, FL Studio e pluginval.

Resultado esperado:

- O plugin passa a se comportar como produto comercial, nao apenas build funcional.

## Backlog Priorizado

### P0

- Reestruturar Main tab em controles musicais primarios.
- Reduzir paleta neon e definir identidade visual.
- Implementar `Save As...` ou remover botao ate estar pronto.
- Melhorar Bypass/Reverse/Freeze com estados inequivocos.

### P1

- Preset browser com categorias, favoritos e edited state.
- Tap editor visual.
- Resize/scaling.
- Tooltips musicais para parametros tecnicos.

### P2

- Medidor/visualizador refinado e opcional.
- A/B compare, undo/redo.
- Skins ou modo compact/expanded.
- Microinteracoes: hover, focus, drag precision, shift-drag fine.

## Criterios de Aceite

- Um usuario novo identifica Time, Feedback e Mix em ate 3 segundos.
- Um usuario consegue carregar preset, alterar cor/movimento e salvar variacao sem manual.
- A interface principal nao expoe mais que 10-12 decisoes simultaneas.
- O visual permanece legivel em 100%, 125% e 150% de escala.
- Estados destrutivos ou dramaticos, como Bypass, Freeze e Reverse, sao obvios.
- A aba avancada existe, mas nao domina a primeira experiencia.
- O plugin comunica claramente: delay digital primitivo, musical, sujo quando desejado, profissional no acabamento.

## Primeiro Sprint Sugerido

1. Renomear e limpar `LookAndFeel`.
2. Criar design tokens e componentes basicos.
3. Redesenhar header e Main tab.
4. Trocar paleta neon por faceplate premium.
5. Implementar `Save As...` minimo com dialog.
6. Adicionar estado visual de preset editado.

Este sprint ja deve mudar radicalmente a percepcao do produto sem tocar no DSP.
