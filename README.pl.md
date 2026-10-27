# Immergas Magis Pro / Combo — emulator Dominusa / panelu na magistrali D+/D- Modbus (ESPHome, M5 Atom)

🇬🇧 [English](README.md) | 🇵🇱 Polski

<p align="center">
  <img src="https://img.shields.io/github/v/tag/jotuera/immergas-magis-pro-combo-dd-modbus?label=version&color=blue" alt="Wersja">
  <img src="https://img.shields.io/badge/platform-ESP32-41BDF5.svg" alt="ESP32">
  <a href="LICENSE"><img src="https://img.shields.io/badge/license-MIT-green.svg" alt="Licencja: MIT"></a>
</p>

Steruj kotłem **Immergas Magis Combo** z **Home Assistanta**, **emulując zdalny sterownik Dominus i panele stref** na magistrali **D+/D- (Modbus RS485)** — za pomocą **M5Stack Atom + ESPHome**. Bez chmury, bez oryginalnego sterownika Dominus.

Kocioł jest Modbusowym **masterem**; to urządzenie jest **slave'em**, który odpowiada jako Dominus (`0x33`) i/lub panele stref (`0x29` / `0x2A` / `0x2B`). Podaje kotłowi temperaturę i wilgotność pokojową z **dowolnego sensora Home Assistanta** i udostępnia ustawienia panelu/Dominusa jako encje HA.

> ⚠️ **Testowane na Magis Combo V2 (`MPROCOMBOV2`)**, a według zgłoszenia użytkownika działa też na **Magis Pro 9 V2** (D+/D- z Dominusem / panelem zdalnym; encje generatora gazowego dotyczą tylko Combo). Aplikacja Dominus obsługuje wiele silników Immergasa (Zeus, Star, VictrixMaior, …) z różnymi mapami rejestrów — ten projekt celuje **wyłącznie w Magis Combo / Magis Pro**. Inne modele mogą działać częściowo, ale nie są testowane. Kontrybucje mile widziane.

## Co nowego w v2.0.0

**Jeden schemat nazewnictwa dla całej rodziny Immergas.** Nazwy plików, identyfikatorów i podstawień zaczynają się teraz od `dd_`, tak jak `tt_` w [tt-bms](https://github.com/jotuera/immergas-magis-pro-combo-tt-bms), żeby configi D+/D-, T-/T+ i F1/F2 mogły w przyszłości działać razem na jednym urządzeniu bez kolizji.

- Pakiety: `packages/dd_core.yaml` … `dd_diagnostics.yaml`; języki: `packages/lang/dd_en.yaml`, `dd_pl.yaml`, `dd_it.yaml`; komponent `dd_bus_climate` (wcześniej `bus_climate`).
- Podstawienia: `dd_temp_src_*` / `dd_hum_src_*`, `dd_uart_*_pin`, `dd_setpoint_step`, nazwy `n_dd_<id>`, teksty `t_dd_*`.
- Ustawienia urządzenia (esp32, logger, debug, sensory uptime / Wi-Fi / czasu pętli / pamięci / przyczyny resetu) przeniesione do głównego YAML-a.
- ⚠️ **Aktualizacja z 1.x:** weź nowy główny YAML i przepisz do niego swoje wartości (zmień `temp_src_*` → `dd_temp_src_*` itd.). **Nazwy encji się nie zmieniają**, więc Home Assistant zachowuje te same encje i historię. Wartości zapamiętane przez urządzenie (nastawy, offsety, tryb, …) są przenoszone jednorazowo ze starych identyfikatorów przy pierwszym uruchomieniu 2.0.0.

## Co nowego w v1.6.0

- **Języki.** Wybierz jeden plik na liście `packages:`: `packages/lang/dd_en.yaml` (domyślny), `dd_pl.yaml` (polski) lub `dd_it.yaml` (włoski — teksty i opisy błędów; nazwy encji wciąż po angielsku, pomoc mile widziana, zob. [TRANSLATING.md](TRANSLATING.md)). ⚠️ Zmiana języka zmienia nazwy encji, więc Home Assistant tworzy nowe.
- Opisy błędów po angielsku, włosku **i polsku**.
- Nowość: **A11 Heat-pump module present**, faza strefy **Air purge** (U50), 7 jeszcze nierozszyfrowanych rejestrów jako domyślnie wyłączone sensory diagnostyczne, podstawienie `setpoint_step`.

## Co nowego w v1.5.0

- **Konfiguracja podzielona na pakiety** (`packages/*.yaml`: core, engine, zone1, zones23, dhw, dominus, schedules, heat_pump, boiler_info, diagnostics). Twój YAML ma teraz ok. 100 linii: nazwa urządzenia, Wi-Fi, dane dostępowe, źródła temperatur i blok `packages:`, który pobiera resztę z tego repo. Rozwiązana konfiguracja jest **identyczna z v1.4.0** (sprawdzone `esphome config`), więc nic się nie zmienia na urządzeniu ani w Home Assistant.
- Piny RS485 są podstawieniami (`uart_tx_pin`, `uart_rx_pin`, `uart_de_pin`).
- Changelog przeniesiony do [CHANGELOG.md](CHANGELOG.md).
- Przygotowanie pod tłumaczenia nazw encji (kolejne wydanie).

## Co nowego w v1.4.0

Dzięki **Roberto Pegurri ([@robpegurri](https://github.com/robpegurri))** za [PR #1](https://github.com/jotuera/immergas-magis-pro-combo-dd-modbus/pull/1), przetestowane na Magis Combo V2 z fizycznym panelem strefy + emulowanym Dominusem, a tutaj z emulowanym panelem + fizycznym Dominusem.

- **Poprawka: zmiany zrobione na fizycznym panelu były cofane.** Kocioł przekazuje je do Dominusa zapisami FC16, które emulator odrzucał. Zmiany harmonogramu widziane na magistrali teraz też aktualizują encje harmonogramu w HA.
- **Arbitraż magistrala / HA** dla nastaw, offsetu, poziomów Comfort/Eco, wilgotności, Set Flow, nastawy CWU i trybu: sprawdzanie zakresów, rozgrzewka po ponownym podłączeniu panelu (koniec z CWU 10°C / offsetem 0 po zaniku zasilania panelu), polecenia HA wstrzymywane do potwierdzenia przez kocioł. Poprawka anty-legionella z v1.2.0 zachowana.
- **Kierunek RS485 (DE) sterowany sprzętowo przez UART** (`flow_control_pin`): koniec z aktywnym oczekiwaniem po każdej odpowiedzi, mniej więcej połowa czasu pętli, 0 błędów CRC.
- **Climates oparte na magistrali**: stan, akcja i presety pochodzą z tego, co raportuje kocioł. ⚠️ **Zmiana łamiąca:** stary preset `Dom` zniknął; presety to **Comfort / Eco** (Manual = brak presetu).
- **Teksty błędów po angielsku lub włosku** (select "Fault language").
- **Diagnostyka**: uptime, siła sygnału Wi-Fi, czas pętli, wolna pamięć, przyczyna resetu, błędy CRC RS485, ramki TX do Dominusa, dryf zegara kotła względem HA, zasilanie/powrót/ΔT systemu, żądanie grzania Strefy 1.
- **Tylko jeden plik do instalacji.** Komponenty projektu (`bus_climate`, `immergas_dd`) pobierają się z tego repo przez `external_components`; nie trzeba niczego kopiować obok YAML-a.
- Drobne poprawki: sensory całkowite pokazują `1` zamiast `1.0`; `HP Night function (U11)` przemianowane na **`HP U11 Night function`** (nowa encja w HA); `Zone 1 heating request` pokazuje on/off.

## Co nowego w v1.3.1

- **Funkcja nocna (parametr kotła U11)** rozszyfrowana na trzech magistralach. W oknie U12–U13 kocioł wysyła do płytki interfejsu jednostki zewnętrznej limit mocy sprężarki w % (`0x1E` rej. `2`, **30** = funkcja nocna; MIM odbija to w rejestrze wejściowym `12`). Jednostka zewnętrzna Samsunga ogranicza wtedy sprężarkę do ~19 Hz zamiast 64 Hz (NASA `0x8239`).
- Nowe encje tylko do odczytu: **HP U11 Night function** (nazwane *HP Night function (U11)* w v1.3.1) oraz **HP Compressor capacity limit** (%, 100 = brak limitu). Pokazują, kiedy limit faktycznie działa, nie tylko czy U11 jest włączone.
- Funkcji nocnej **nie da się przełączyć z Home Assistanta**: zapisuje ją tylko kocioł, a to urządzenie nigdy nie wysyła nic do `0x1E`. Ustaw U11 / U12 / U13 w menu kotła.
- `Boiler firmware version` przemianowane na **`D91 Boiler firmware version`** (ta sama wartość co D91 na magistrali T-/T+ BMS).

## Co nowego w v1.3.0

Ustawienia panelu strefy rozszyfrowane przez przeklikanie każdego menu fizycznego panelu podczas podsłuchu magistrali. Kocioł **czyta** je z panelu `0x29`, więc emulowany panel może teraz ustawiać je z Home Assistanta (dotąd były zahardkodowane):

| Rejestr | Encja | Wartości | Domyślnie |
|---|---|---|---|
| `2294` | **Time programs Zone 1** (select) | Manual / Auto | Manual |
| `6105` | **Room probe modulation** (select) | NO / YES | NO |
| `6107` | **Dew point correction** (switch) | on / off | on |

- Wartości domyślne odpowiadają poprzednim zahardkodowanym wartościom, więc **po aktualizacji nic się samo nie zmienia**, dopóki ich nie dotkniesz.
- **Auto** sprawia, że kocioł podąża za tygodniowym harmonogramem CO (comfort / economy). Przy *Manual* — poprzednim zachowaniu — harmonogram był ignorowany podczas emulacji panelu strefy. Sprawdź swoją nastawę economy przed przełączeniem na Auto.
- Przy podłączonym fizycznym panelu kontrolki odzwierciedlają jego własne odpowiedzi (panel jest wtedy źródłem prawdy).
- Nowe encje tylko do odczytu: **A31 Zone 1 room thermostat** (`6102`, RPT / RT / RP), **Time program state (boiler)** (`2090`: Manual / Auto (comfort) / Auto (economy)), **D09 DHW request pending** i **Boiler fault active (2001)**.
- **A31 nie da się ustawić przez magistralę**: to parametr kotła, który kocioł zapisuje do panelu, ale nigdy nie odczytuje z powrotem. Zmień go w menu kotła.
- W ogóle nietransmitowane (lokalne dla panelu lub nieobsługiwane przez kocioł): temperatura przeciwzamrożeniowa, cykl antybakteryjny panelu, wyłączenie odwilżania, minimalna nastawa chłodzenia, program urlopowy.

Pełna lista rejestrów, które kocioł czyta z panelu: [REGISTER_MAP.md](REGISTER_MAP.md).

## Co nowego w v1.2.0

> ⚠️ **Aktualizacja zalecana dla wszystkich użytkowników v1.0/v1.1 emulujących panel strefy.** Jeśli funkcja **anty-legionella** kotła jest włączona (parametr **P15**, domyślnie co poniedziałek w nocy), starsze wersje mają **nastawę CWU zablokowaną na 60°C** po cyklu dezynfekcji — widoczne wszędzie (HA, Dominus, kocioł), dopóki nie zmienisz ręcznie. Przyczyna: emulator przyjmował *aktywną* nastawę CWU kotła (`3015`, zapisywaną do panelu) jako zmianę użytkownika i, działając jako panel, odsyłał 60°C z powrotem do kotła. Naprawione: tylko odpowiedzi urządzenia (Dominus / fizyczny panel) zmieniają nastawę CWU; aktywna nastawa kotła jest pokazywana tylko do odczytu jako **`D05 Active DHW setpoint (boiler)`**.

- **Dane pompy ciepła** rozszyfrowane z płytki interfejsu jednostki zewnętrznej (`0x1E`, Samsung MIM) — częstotliwość sprężarki, temperatury tłoczenia / parownika / obudowy sprężarki, prąd sprężarki, zadane obroty wentylatora, pozycja EEV, zadana wody, flagi pracy (`Run request`, `Request accepted`, `Compressor running`, `Fan running`), etap pracy, wersje firmware. Zweryfikowane krzyżowo na trzech magistralach (D+/D-, T-/T+ BMS i Samsung NASA F1/F2).
- **Dane kotła** z bloku kocioł → panel: wyliczona nastawa zasilania **D04**, zasilanie i powrót generatora głównego / dodatkowego (pompa ciepła / kocioł gazowy), zegar kotła, liczba stref **A13**.
- Nazwy encji zawierają **kody z instrukcji Immergas** (`D71`, `D73`, `D120-D123`, …).
- Opcjonalny **sniffer magistrali** (przełącznik diagnostyczny, **domyślnie WYŁĄCZONY**) — loguje każdą zmianę rejestru z tagiem `sniff`; przydatny przy zgłaszaniu błędów i rozszyfrowywaniu nowych rejestrów.
- Pełna mapa rejestrów: **[REGISTER_MAP.md](REGISTER_MAP.md)** — pomoc z pozostałymi nieznanymi rejestrami mile widziana.

> v1.2.0 było testowane na **jednostrefowym Magis Combo V2**. Nowe encje pompy ciepła powinny działać też na Magis Pro (ta sama jednostka zewnętrzna Samsunga + płytka MIM); encje generatora gazowego dotyczą tylko Combo. Instalacje wielostrefowe nie zostały ponownie przetestowane z v1.2.0 — opinie mile widziane.

## Co robi

- **Emuluje Dominusa** (`0x33`) i/lub **panele stref** (`0x29` = Strefa 1, `0x2A` = Strefa 2, `0x2B` = Strefa 3), żeby kocioł działał bez oryginalnego sterownika.
- **Podaje kotłowi temperaturę i wilgotność pokojową** z sensorów HA — wybierz spośród do 5 przełączalnych źródeł na strefę (Panel / Piwnica / Parter / Piętro / Cały dom). "Panel" używa prawdziwej wartości odczytanej z fizycznego panelu na magistrali, jeśli jest obecny.
- **Steruje**: trybem pracy, nastawą CO strefy, offsetem grzania (U03/U04/U16), nastawami Comfort/Eco grzania i chłodzenia, set-flow, ustawieniem wilgotności (U07/U08/U18), nastawą CWU.
- **Harmonogramy tygodniowe (chrono)**: 4 wspólne kalendarze profili czasowych (`Kalendarz 1–4`) + niezależne przypisanie dzień→kalendarz dla każdej strefy i dla CWU.
- **Monitoruje**: kod błędu kotła + **114 opisów błędów po angielsku zgodnie z oficjalną terminologią Immergas** (wyciągnięte z etykiet aplikacji Dominus), fazę każdej strefy (Comfort/Eco), status połączenia każdego urządzenia, wersje firmware.
- **Funkcja nocna** (v1.3.1): flaga aktywności U11 i limit mocy sprężarki (%) z płytki interfejsu jednostki zewnętrznej (tylko do odczytu).
- **Ustawienia panelu strefy** (v1.3.0): programy czasowe Manual/Auto, modulacja sondy pokojowej, korekcja punktu rosy (sterowalne); termostat pokojowy A31, stan programu czasowego, oczekujące żądanie CWU i flagi błędów (tylko do odczytu).
- **Pompa ciepła i generatory** (v1.2.0): Hz / prąd / temperatury sprężarki, wentylator, EEV, flagi pracy i etap z płytki interfejsu jednostki zewnętrznej; temperatury zasilania/powrotu pompy ciepła i kotła gazowego; wyliczona nastawa zasilania (D04); aktywna nastawa CWU; zegar kotła.
- **Lokalny interfejs WWW** (serwer WWW ESPHome, chroniony hasłem) jako zapasowy, niezależny od Home Assistanta.

## Sprzęt

- **M5Stack Atom Lite** (ESP32) + izolowany transceiver **RS485** (np. M5 Isolated RS485 Unit / MAX3485).
- Piny: **TX = GPIO26, RX = GPIO32, DE = GPIO23**.
- Podłączenie: Immergas **D+ / D-** → RS485 **A / B** (zamień, jeśli dostajesz błędy CRC).
- Magistrala: **Modbus RTU, 9600 8E1** (parzystość even, 2-przewodowa).
- **Rezystor terminujący 120 Ω między A/B jest wymagany**, żeby Atom działał samodzielnie na magistrali (kocioł zapewnia polaryzację; brakuje tylko terminacji). Zestaw M5 Isolated RS485 ma go w komplecie.

## Jak działa emulacja (ważne)

Emulujesz **albo Dominusa, albo panele — nigdy ten sam adres co fizyczne urządzenie** na magistrali (dwa urządzenia odpowiadające na jeden adres = kolizja). Przełączniki:

- `Emulate Dominus` (0x33), `Emulate Panel Zone 1/2/3` (0x29 / 0x2A / 0x2B) — wzajemnie wykluczające się z fizycznym urządzeniem na tym adresie.
- `Emulate Modbus` — master wł./wył. (zatrzymuje cały RX/TX).
- **Zabezpieczenie przed kolizją:** nie da się włączyć emulacji adresu, na którym już odpowiada fizyczne urządzenie (sensor połączenia pokazuje "connected"). Najpierw wyłącz fizyczne urządzenie.
- Adres **`0x1E`** to **płytka interfejsu jednostki zewnętrznej** (Samsung MIM, "A22" na schemacie, status połączenia = kod z instrukcji **D62**) — **prawdziwe urządzenie**. Jest tylko odczytywane (dane pompy ciepła) i **nigdy nie wysyła się do niego** (twarde zabezpieczenie w kodzie).

Typowe konfiguracje:
- **Jest fizyczny Dominus** → emuluj panel (Strefa 1), żeby podawać temperaturę pokojową; emulację Dominusa zostaw wyłączoną.
- **Brak Dominusa** → emuluj Dominusa (0x33), żeby przejąć tryb pracy i ustawienia.

## Mapa rejestrów (D+/D-, znane)

| Rej | Kod | Opis | Skala |
|---|---|---|---|
| 2000 | — | Tryb pracy (0=standby, 1=lato/CWU, 2=chłodzenie, 3=zima) | RW |
| 2005 / 2011 / 3009 | — | Temperatura pokojowa strefy | ×0,1°C |
| 2006 / 3010 | D41/D42/D102 | Wilgotność względna strefy | % |
| 2010 | — | Status/faza strefy (bit6=Comfort, bit7=Eco, bit2=Manual) | R |
| 2015 | — | Nastawa CO strefy | ×0,1°C, RW |
| 2017 / 2219 | U03/U04/U16 | Offset grzania strefy | ×0,1°C, RW |
| 2095 | D05 | Nastawa CWU | ×0,1°C, RW |
| 2100 | — | Kod błędu (0 = brak, inaczej kod E) | R |
| 2210 / 2211 | — | Nastawa grzania Comfort / Eco | ×0,1°C, RW |
| 2214 / 2215 | — | Nastawa chłodzenia Comfort / Eco | ×0,1°C, RW |
| 2216 | U07/U08/U18 | Ustawienie wilgotności | %, RW |
| 2217 | — | Temperatura set-flow | ×0,1°C, RW |
| 2300 | D140–D142 | Zegar kotła: [dzień tygodnia 3 bity][godzina 5 bitów][minuta 8 bitów], 1 = pon. (CET, bez czasu letniego) | R |
| 2302 / 2303 | D143–D145 | Data sterownika (dzień.miesiąc / rok) | R |
| 2310–2347 | — | 4 profile kalendarza (po 4 przedziały czasowe, wł./wył.) — bajt wysoki = godzina, niski = minuta | RW |
| 2410–2416 / 2420–2426 / 2430–2436 | — | Dzień → kalendarz dla Strefy 1 / 2 / 3 (wartość 1–4) | RW |
| 2490–2496 | — | Dzień → kalendarz dla CWU (pojedynczy, wspólny obieg domu) | RW |
| 3002 | D06 | Zewnętrzna temperatura otoczenia | ×0,1°C |
| 3003 | D04 | Wyliczona nastawa zasilania (krzywa grzewcza + U03; 0 bez żądania) | ×0,1°C |
| 3011 / 3012 | D20 / D08 | Generator główny (pompa ciepła): zasilanie / powrót | ×0,1°C |
| 3013 / 3014 | D01 / — | Generator dodatkowy (kocioł gazowy): zasilanie / powrót | ×0,1°C |
| 3015 | D05 | **Aktywna** nastawa CWU używana przez kocioł (np. 60 podczas anty-legionelli) | ×0,1°C |
| 3016 | D03 | Temperatura zasobnika CWU | ×0,1°C |
| 3042 | — | Wersja firmware panelu | ×0,01 |
| 4199 (0x33) | A13 | Liczba stref | — |
| 0x1E I3 / I4 / I5 / I6 | D73 / D74 / D79 / D72 | Tłoczenie / parownik / sonda zewnętrzna / obudowa sprężarki | ×0,1°C |
| 0x1E I11 / I13 / I14 / I16 | D71 / D75 / D76 / D77 | Hz sprężarki / prąd (×0,1 A) / zadane obroty wentylatora / pozycja EEV (kroki) | — |
| 0x1E W20 / W21 / W22 | D08 / D20 / D24 | Powrót wody (TW1) / zasilanie (TW2) / czynnik ciekły, wysyłane przez kocioł do pompy ciepła | ×0,1°C |

Adresy: `0x33` Dominus · `0x29` panel Strefy 1 · `0x2A` Strefa 2 · `0x2B` Strefa 3 · `0x1E` płytka interfejsu jednostki zewnętrznej (prawdziwe urządzenie, tylko do odczytu). Pełna mapa z kierunkami, pewnością i nieznanymi: **[REGISTER_MAP.md](REGISTER_MAP.md)**.

## Konfiguracja

Edytuj blok `substitutions:` na początku YAML-a — wskazują na **Twoje** sensory Home Assistanta:

- `dd_temp_src_*` / `dd_hum_src_*` — źródła temperatury/wilgotności Strefy 1 (sloty 2–5; slot 1 "Panel" = żywa wartość z magistrali).
- `dd_temp_src_z2_*` / `dd_hum_src_z2_*`, `dd_temp_src_z3_*` / `dd_hum_src_z3_*` — źródła Strefy 2 / 3.
- `dd_uart_tx_pin`, `dd_uart_rx_pin`, `dd_uart_de_pin` — piny RS485 (domyślnie GPIO26 / GPIO32 / GPIO23 dla M5Stack Atom); dodaj tylko jeśli Twoje okablowanie jest inne.
- Każdą inną wartość pakietów można nadpisać w ten sam sposób (np. pojedynczą nazwę encji w kolejnym wydaniu).

## Instalacja

1. Skopiuj `immergas-magis-pro-combo-dd-modbus.yaml` do folderu konfiguracji ESPHome. Reszta (pliki `packages/dd_*.yaml` i komponenty `dd_bus_climate` / `immergas_dd`) pobiera się z GitHuba. Żeby zaktualizować, zmień `ref:` w bloku `packages:` (albo użyj `main` dla wersji rozwojowej). Trzymaj kompletną listę plików pakietów: pakiety zależą od siebie nawzajem.
1. `cp secrets.yaml.example secrets.yaml` i uzupełnij dane WiFi + dostęp do interfejsu WWW.
2. Skieruj źródła w `substitutions:` na swoje sensory HA (patrz wyżej).
3. Wgraj `immergas-magis-pro-combo-dd-modbus.yaml` przez ESPHome, podłącz RS485 (z rezystorem **120 Ω**), dodaj w Home Assistant.

## Uwagi

- **Wspólne kalendarze:** 4 profile czasowe są wspólne dla wszystkich stref i CWU; tylko przypisanie dzień→kalendarz jest per-strefa (potwierdzone z configu aplikacji Dominus).
- **CWU to pojedynczy obieg** — jeden harmonogram ciepłej wody dla całego domu (bez CWU per strefa).
- **Opisy błędów kotła** to oficjalne angielskie teksty Immergas (z etykiet aplikacji Dominus), nie tłumaczenie maszynowe.

## Zastrzeżenie

Zrewersowane na potrzeby **interoperacyjności / użytku prywatnego**. Niezwiązane z Immergasem ani przez niego nieautoryzowane. Emulowanie sterownika i zapisywanie do rejestrów zmienia zachowanie urządzenia — **używasz na własną odpowiedzialność**.

## Licencja

MIT — zob. [LICENSE](LICENSE).
