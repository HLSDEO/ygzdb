# Cartas do Destiny (PS1)

Jogo de duelo de cartas em turnos para PlayStation 1, inspirado no estilo de
*Yu-Gi-Oh! Forbidden Memories* (sem 3D animado — batalha de cartas 2D).

## Como funciona

- 8000 LP por duelistão
- Deck de 30 cartas, mão inicial de 5
- 18 tipos de monstros (ATK/DEF, atributo: Fogo, Água, Terra, Vento, Luz, Trevas)
- 5 zonas de monstro por lado
- Turnos: Draw → Main → Battle → End
- Summon (X) em ataque ou Set (△) em defesa face-down
- IA do CPU joga o monstro mais forte e ataca o alvo mais fraco

## Controles

| Botão | Ação |
|-------|------|
| ← →   | Navegar mão / ciclar atacantes / alvos |
| ↑ ↓   | Ciclar atacantes / alvos |
| X     | Summon (ataque) / atacar alvo selecionado |
| △     | Set (defesa face-down) |
| ○     | Fim da fase (Main → Battle → End) |
| START | Iniciar duelo / voltar ao título |

## Build (Docker)

Requisitos: Docker Desktop

```powershell
.\build.ps1            # build completo + ISO
.\build.ps1 -Shell     # shell interativo no container
```

Saída: `CARTAS.iso` na raiz do projeto.

## Testar

Abra `CARTAS.iso` em:

- **DuckStation** (recomendado)
- **PCSX-Redux**
- **Mednafen**

## Estrutura

```
src/
  main.c         # entry, título, loop principal
  gfx.c/h        # renderizador software 2D (VRAM double-buffer)
  font8x8.h      # fonte bitmap 8x8
  cards.c/h      # banco de cartas + deck
  duel.c/h       # regras do duelo + IA
  duel_render.c  # UI do duelo
iso_root/
  SYSTEM.CNF
CMakeLists.txt   # psn00bsdk_add_executable
iso.xml          # mkpsxiso
Dockerfile       # toolchain psn00bSDK + mkpsxiso
```
