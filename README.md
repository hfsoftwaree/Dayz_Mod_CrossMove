# ✝️ ZHF CrossMove – Controle da Cruz de Túmulo para DayZ Expansion

Mod de **scripts (Enforce Script)** para **DayZ Standalone** que estende o sistema de **cruz de túmulo (Gravecross)** do **DayZ Expansion**. Ele **move a cruz para pontos seguros** quando o jogador morre em áreas restritas, envia um **kill feed para o Discord**, cria um **marcador de morte pessoal** e **registra o loot da cruz em JSON**.

---

## 📦 Dependência

- **DayZ Expansion** (`Expansion_Gravecross`, `Expansion_Gravecross_LowLifetime` e o módulo de marcadores `ExpansionMarkerModule`)

---

## ✨ Funcionalidades

- 📍 **Zonas de restrição:** se a cruz nascer dentro do perímetro de uma zona configurada, ela é movida para um **ponto de salto (Jump Point)** sorteado, com um pequeno deslocamento aleatório (±2 m) para evitar sobreposição.
- ☠️ **Kill feed no Discord:** embed com vítima, assassino, arma, SteamIDs (com link do perfil Steam), coordenadas da cruz e link do iZurvive.
- 🎯 **Captura inteligente da morte:** identifica suicídio, morte por outro jogador (inclusive por item/entidade do jogador), bots/NPCs e dano ambiental.
- 🗺️ **Marcador de morte pessoal:** cria um marcador no mapa do jogador (via Expansion) na posição final da cruz.
- 💾 **Registro de loot em JSON:** salva o conteúdo completo da cruz (itens, attachments e itens aninhados), com dono, posição e expiração.
- 🔧 **Config automática:** o `Config.json` é criado e completado automaticamente na inicialização do servidor.

---

## ⚙️ Como funciona

1. **Morte do jogador** (`PlayerBase.EEKilled`): os dados da morte (vítima, assassino, arma) são guardados em uma fila.
2. **Criação da cruz** (`Expansion_Gravecross` e `Expansion_Gravecross_LowLifetime`): após ~1,2 s, o mod:
   - verifica se a posição está em uma zona de restrição e, se estiver, move a cruz para um Jump Point;
   - retira o próximo registro da fila e envia o kill feed ao Discord;
   - cria o marcador de morte pessoal;
   - salva o loot da cruz em JSON.

---

## 🗂️ Arquivos gerados

Criados no perfil do servidor:

| Caminho | Conteúdo |
|---|---|
| `$profile:\HF\GraveCross\Config.json` | Configuração do mod |
| `$profile:\HF\GraveCross\Loot\<SteamID>_<ExpireEpoch>.json` | Loot de cada cruz |

### Exemplo de `Config.json`

```json
{
    "enable": 1,
    "webhook": "COLE_AQUI_SEU_WEBHOOK",
    "mapname": "chernarusplus",
    "m_RestrictionZones": [
        {
            "Area_Description": "Zona de exemplo",
            "Block_Position": "7500 300 7600",
            "Perimeter": 150.0,
            "Jump_Points": [
                "7700 300 7800",
                "7720 300 7810"
            ]
        }
    ]
}
```

| Campo | Descrição |
|---|---|
| `enable` | Ativa (`1`) ou desativa (`0`) as **zonas de restrição**. Não é criado automaticamente, então o padrão é desativado |
| `webhook` | Webhook do Discord para o kill feed |
| `mapname` | Nome do mapa usado no link do iZurvive |
| `Area_Description` | Descrição livre da zona |
| `Block_Position` | Centro da zona, no formato `"x y z"` |
| `Perimeter` | Raio da zona em metros (distância horizontal) |
| `Jump_Points` | Lista de posições `"x y z"` para onde a cruz pode ser movida |

---

## 📁 Estrutura do projeto

```
ZHF_CrossMove/
├── config.cpp
└── scripts/
    ├── 4_World/
    │   ├── GraveCrossManager (config, zonas, kill feed)
    │   ├── PlayerBase (captura da morte)
    │   ├── Expansion_Gravecross (movimento da cruz)
    │   └── GraveCrossLootLogger (registro de loot)
    └── 5_Mission/
        └── MissionServer (criação do Config.json)
```

> Ajuste os nomes conforme os arquivos reais do seu projeto.

---

## 🧰 Tecnologias

- Enforce Script (DayZ)
- `JsonFileLoader` para persistência
- `RestApi` para webhooks do Discord
- Integração com **DayZ Expansion** (cruz de túmulo e marcadores)

---

## 👤 Autor

Desenvolvido por **HF**.

