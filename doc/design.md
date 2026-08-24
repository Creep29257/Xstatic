# Xstatic — Notes d'architecture

Client lourd Meshtastic en C, interface X11 (Xlib), portable FreeBSD/Linux.

## Découpage en couches

```
src/platform/   -- accès série, dépendant de l'OS
src/protocol/   -- framing + décodage protobuf Meshtastic
src/core/       -- état du mesh (à venir)
src/ui/         -- interface graphique, Xlib (à venir)
```

Règle de dépendance : chaque couche ne connaît que celle en dessous.
`ui/` ne connaît pas `platform/` directement, `core/` ne connaît ni
Xlib ni le port série. Le seul fichier qui assemble tout est `main.c`.

## platform/ — accès série

Interface commune dans `platform.h` :

- `platform_serial_open(device)` — ouvre le device, configure en mode
  brut (`cfmakeraw`) à 115200 bauds via `termios`. Retourne un fd ou -1.
- `platform_serial_read(fd, buf, count)` — wrapper sur `read(2)`.
- `platform_serial_write(fd, buf, count)` — wrapper sur `write(2)`.
- `platform_serial_close(fd)` — ferme le fd.

Chaque OS a sa propre implémentation (`platform_freebsd.c`,
`platform_linux.c`), même interface. Point d'attention portabilité :
le chemin du device diffère (`/dev/cuaU0` sur FreeBSD via le driver
`umodem`, `/dev/ttyACM0` sur Linux).

Sur FreeBSD, le device série appartient au groupe `dialer` — ajouter
l'utilisateur à ce groupe (`pw groupmod dialer -m <user>`) plutôt que
de lancer le client en root.

## protocol/ — framing Meshtastic

### Format de trame (liaison série)

```
[0x94] [0xc3] [longueur MSB] [longueur LSB] [payload protobuf...]
```

- `0x94 0xc3` (START1/START2) — magiques fixes marquant le début
  d'une trame.
- longueur — 2 octets, big-endian, taille du payload qui suit. Limité
à FRAMING_MAX_PAYLOAD (512 octets) côté implémentation : toute
longueur annoncée au-delà est rejetée par framing_feed() (protection
contre débordement de state->payload).
- payload — message protobuf `FromRadio` (device → client) ou
  `ToRadio` (client → device).

### Comportement du firmware sur le port série

Par défaut, le firmware imprime ses logs de debug en texte lisible
(ANSI, couleurs) sur le port série. Il ne bascule en émission de
trames protobuf structurées qu'après avoir reçu une trame `ToRadio`
valide de la part du client — les deux flux (texte debug + binaire
protobuf) peuvent ensuite être entrelacés sur le même port.

Handshake minimal utilisé pour déclencher la réponse : un `ToRadio`
contenant uniquement le champ `want_config_id` (numéro de champ 3,
encodé varint) :

```
94 c3 00 02 18 01
```

(`18` = tag du champ 3 en varint, `01` = valeur du want_config_id)

### `framing_feed()` — streaming parser

`framing.h`/`framing.c` implémentent un parser incrémental : il ne
connaît pas la source des octets (série, TCP, fichier...), il reçoit
juste des octets "poussés" au fur et à mesure (`framing_feed(state,
buf, count)`) et maintient son état entre les appels via
`struct framing_state`, passée par pointeur. Nécessaire parce qu'un
`read()` sur le port série peut couper une trame n'importe où — le
parser doit pouvoir reprendre correctement au prochain appel.

Machine à états (`enum framing_phase`) :

```
WAIT_START1 → WAIT_START2 → READ_LEN_HI → READ_LEN_LO → READ_PAYLOAD → (retour à WAIT_START1)
```

Chaque octet du buffer d'entrée est traité un par un dans une boucle
`for`; le `switch` sur `state->phase` décide de l'action à faire et
du changement d'état éventuel.

## Conventions de code

- Préfixe = nom du module sur toute fonction/type exposé
  (`platform_*`, `framing_*`, `mesh_state_*`, `ui_*`).
- snake_case partout, y compris les types (`mesh_state_t`, pas
  `MeshState`).
- Header guards `#ifndef MODULE_H` / `#define` (pas `#pragma once`).
- Types opaques pour l'état interne quand possible (struct définie
  dans le `.c`, seul un typedef exposé dans le `.h`) — pas encore
  appliqué à `framing_state`, qui doit être visible du `.h` pour que
  `main.c` puisse la déclarer sur la pile.
- `cc` invoqué en direct dans les commandes/Makefile plutôt que `gcc`
  ou `clang` en dur, pour rester portable FreeBSD/Linux.

## Prochaines étapes (ordre révisé v0.1)

- ✅ `core/mesh_state.c` : fait.
- ✅ Envoi (`ToRadio` → protobuf → framing → port série) : fait, compile,
  pas encore testé sur device réel. Voir détail ci-dessous.
- Mapper les tags FromRadio restants (config, moduleConfig, channel,
  metadata, queueStatus...) à des affichages utiles.
- `ui/ui_xlib.c` : différé après l'envoi (inversion de priorité v0.1 —
  un client CLI complet, lecture + écriture, passe avant l'interface
  graphique).
- Fusion `select()` : reste le point le plus délicat, nécessaire pour
  l'UI (fd Xlib + fd série) et pour un éventuel mode envoi+écoute
  simultané côté CLI (actuellement l'envoi est fire-and-forget : le
  programme envoie et quitte, sans lire la réponse).

## protocol/ — construction ToRadio (envoi)

Symétrique du décodage `FromRadio`, dans `main.c` :

- `to_radio_construct(char *to_str, char *message, meshtastic_ToRadio *out)`
  remplit `ToRadio.packet` (`to` via `strtoul`, `from=0`, `channel=0`) et
  `packet.decoded` (`portnum=TEXT_MESSAGE_APP`, `payload` via `memcpy` avec
  garde sur la taille — `Data.payload` est un `PB_BYTES_ARRAY_T(233)`, pas
  null-terminé). Paramètre de sortie par pointeur, retour `int` (0/-1),
  même pattern que `framing_handshake_construct`.
- `to_radio_encode(meshtastic_ToRadio *to_radio, uint8_t *out_buffer, size_t *out_len)`
  appelle `pb_encode` sur `meshtastic_ToRadio_fields` et récupère la taille
  réelle via `stream.bytes_written`.
- `framing_message_construct(unsigned char *payload, size_t payload_len, unsigned char *out_frame, size_t out_frame_len)`
  (dans `framing.c`, symétrique de `framing_feed`) emballe un payload de
  taille variable avec magic bytes + longueur 2 octets big-endian.

Résolution du destinataire par num uniquement pour l'instant — la
résolution par nom (`long_name`) nécessiterait que `mesh_state` soit
peuplé avant l'envoi, ce qui suppose une phase de lecture préalable
(actuellement absente du mode envoi).

## Licence

BSD 2-clause, voir `LICENSE`.
