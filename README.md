<<<<<<< HEAD
# Grogu AI Assistant

Turning an animatronic Grogu plush into a voice-controlled AI assistant, connected to real tools.

The goal is simple: touch his head, talk to him, and get an answer in about a second, while his ears, eyelids and arm move as he speaks. Beyond conversation, he becomes a physical companion for everyday work: focus timers, morning briefings, business alerts, and smart home actions.

The whole build is documented step by step in a short video series, and everything needed to reproduce it lives in this repository.

## How it works

The plush keeps its original mechanics: a single DC motor with a cam gearbox, a small speaker, and a capacitive touch plate on top of the head. The original control board is replaced by an ESP32-S3.

The ESP32 stays deliberately simple. It captures audio, plays the voice, drives the motor, reads the sensors, and talks to a server over WebSocket. All the intelligence runs on the server: speech-to-text, the language model, text-to-speech, tool calling, MCP integrations, webhooks and alerts.

```
Touch -> ESP32 (mic) -> Server: speech-to-text -> LLM + tools -> text-to-speech -> ESP32 (speaker + motor)
```

## Repository structure

| Folder | Content | Status |
| --- | --- | --- |
| [`arduino/motor`](arduino/motor) | First test: driving the original motor with an Arduino and an L293D | Available |

More folders will be added as the build progresses: ESP32 firmware, server, AI pipeline, and the control interface.

## Roadmap

- [x] Teardown and component identification
- [x] Motor control with an Arduino
- [ ] ESP32-S3 brain transplant (mic, amp, motor driver, touch sensor)
- [ ] First words through the server pipeline
- [ ] Latency optimization with end-to-end streaming
- [ ] Motor movement synced with speech
- [ ] Webhook endpoint for spoken alerts
- [ ] Tool calling (pomodoro timer, calendar, weather)
- [ ] Control interface
- [ ] Extra sensors: motion, accelerometer, light, RFID mode cards

## Safety principles

- The microphone only listens when his head is touched.
- Transcripts stay on the local server.
- Tools are read-only at first; any action requires explicit confirmation.
- Everything runs on 5 V USB. No mains voltage ever goes inside the plush.

---

Fan-made project. Not affiliated with, endorsed or sponsored by Disney or Lucasfilm.
=======
# TableAgent · plateforme interne (V1)

Outil interne de l'équipe TableAgent : clients, onboarding restaurant par restaurant, support.

- `frontend/` : application React
- `backend/` : API Fastify et base PostgreSQL (schéma, migrations, données de départ)
- `shared/` : types et règles communs au front et au back
- `docs/` : specs (PRD, specs fonctionnelles, modèle de données, rôles, intégrations Google, maquettes, architecture)
- `prototype/` : la maquette cliquable, qui fait foi pour les écrans et les libellés
- `Dockerfile`, `deploy/compose.yaml` : la production en conteneurs Docker (voir [docs/DEPLOIEMENT.md](docs/DEPLOIEMENT.md)) ; `.github/workflows/` : la CI

## Démarrer

```bash
nvm use && corepack enable && pnpm install
docker compose up -d db
pnpm db:migrate && pnpm db:seed
pnpm dev        # http://localhost:5173
```

Aucun fichier `.env` n'est nécessaire en développement. Sans identifiants Google, la page de connexion propose de choisir une personne de l'équipe.

Conventions, commandes et variables d'environnement : voir [CLAUDE.md](CLAUDE.md).
>>>>>>> 071245ea61483bc79917058053c17318900a2f1d
