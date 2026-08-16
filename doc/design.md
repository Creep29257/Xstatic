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
- longueur — 2 octets, big-endian, taille du payload qui suit
  (max observé dans le client officiel : 512 octets).
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

## Prochaines étapes (au moment de la rédaction)

- Brancher `framing_feed()` dans `main.c` à la place du dump hexa brut.
- Intégrer nanopb pour décoder le payload une fois une trame complète
  détectée (`FromRadio`, `NodeInfo`, `MyNodeInfo`...).
- `core/mesh_state` : état du mesh (liste de nodes), logique métier
  pure sans dépendance OS/UI.
- `ui/ui_xlib.c` : fenêtre Xlib, boucle d'événements, fusion avec le
  fd série via `select()`.

## Licence

BSD 2-clause, voir `LICENSE`.
