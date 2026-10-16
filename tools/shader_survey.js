// Evaluated in the page by N64Recomp-browser's tools/record_shaders.mjs (tools/shader_plan.json) and play_page.mjs
// (tools/play_plan.json), with n64r and n64w, RDRAM words at N64 addresses (the US 1.1 ROM's, from the sf64
// decompilation). Offers the game's own ways to a level, through patches/shader_survey.c's request in RDRAM:
//
//   level(id, phase)  starts a level the way the game's own state change does (gNextGameState), with the player kept
//       alive (`invincible`): 0 Corneria, 1 Meteo, 2 Sector X, 3 Area 6, 5 Sector Y, 6 Venom 1, 7 Solar, 8 Zoness,
//       9 Venom (Andross), 10 Training, 11 Macbeth, 12 Titania, 13 Aquas, 14 Fortuna, 16 Katina, 17 Bolse, 18 Sector Z,
//       19 Venom 2; phase 1 is Meteo's and Sector X's warp zone. Resolves once the level is playing.
//   gameState(state)  switches to a game state: 3 the main menu, 4 the Lylat map, 5 game over, 6 Versus, 8 the ending.
//   untilState(state, ms)  resolves once the game is in that state (2 the title), or after ms.
//   fireFor(ms)  sets window.firing to ms from now, which a pressUntil of A fires until.
//   state()  the game state, the level and its phase, for the console.
(() => {
  const word = (a) => n64r(a & ~3) >>> 0;
  const rh = (a) => (word(a) >>> (16 - 8 * (a & 2))) & 0xffff;
  const sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms));
  const until = async (test, ms) => {
    const end = Date.now() + ms;
    while (!test() && Date.now() < end) await sleep(50);
    return test();
  };

  const gGameState = 0x80177834, gCurrentLevel = 0x80178234, gLevelPhase = 0x8017827C, gNextGameState = 0x80161A32;
  const gPlayState = 0x80177854;
  const GSTATE_PLAY = 7;

  // patches/shader_survey.c's struct: magic, magic, taken, state, level, phase, invincible.
  const survey = () => {
    if (window.__survey === undefined) {
      for (let a = 0x80801000; a < 0x81000000; a += 4) {
        if (n64r(a) === 0x53484452 && n64r(a + 4) === 0x53525659) { window.__survey = a; break; }
      }
    }
    return window.__survey;
  };

  window.state = () => `state ${word(gGameState)} next ${rh(gNextGameState)} level ${word(gCurrentLevel)} ` +
    `phase ${word(gLevelPhase)} play ${word(gPlayState)}`;

  const request = async (state, level = -1, phase = 0) => {
    const s = survey();
    if (s === undefined) return 'no survey struct in RDRAM';
    const taken = n64r(s + 8);
    n64w(s + 16, level);
    n64w(s + 20, phase);
    n64w(s + 24, 1);
    n64w(s + 12, state);
    return (await until(() => n64r(s + 8) !== taken, 5000)) ? null : 'request not taken: ' + window.state();
  };

  window.level = async (id, phase = 0) => {
    const failed = await request(GSTATE_PLAY, id, phase);
    if (failed) return failed;
    const ok = await until(() => word(gGameState) === GSTATE_PLAY && word(gCurrentLevel) === id, 30000);
    return (ok ? 'playing: ' : 'no level: ') + window.state();
  };

  window.gameState = async (state) => {
    const failed = await request(state);
    if (failed) return failed;
    const ok = await until(() => word(gGameState) === state, 30000);
    return (ok ? 'in: ' : 'not in: ') + window.state();
  };

  window.untilState = async (state, ms) =>
    ((await until(() => word(gGameState) === state, ms)) ? 'in: ' : 'not in: ') + window.state();

  window.fireFor = (ms) => {
    window.firing = Date.now() + ms;
    return `firing for ${ms / 1000} s`;
  };
})();
