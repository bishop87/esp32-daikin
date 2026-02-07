# O — Orchestration

## Development Order (Mandatory)

1. Analisi protocollo S21
2. Implementazione driver S21 (lettura)
3. Modellazione stato interno (DaikinState)
4. Scrittura S21 (comandi base)
5. Stabilizzazione driver
6. Implementazione API REST
7. Gestione WiFi
8. Web UI
9. Refinement e debug

NON è consentito cambiare l’ordine.

---

## Decision Rules

- Nessun modulo può accedere direttamente a GPIO eccetto s21_driver
- Nessuna API REST può comunicare direttamente con S21
- La Web UI non può contenere logica di business
- DaikinState è l’unica fonte di verità dello stato

---

## Validation Rules

- Prima di scrittura S21 → lettura stabile
- Prima di Web UI → API REST complete
- Ogni comando deve essere verificabile via stato letto

---

## Error Handling Strategy

- Logging seriale dettagliato per S21
- Nessun crash su frame malformati
- Timeout gestiti senza blocchi

---

## Iteration Strategy

- Implementazione incrementale
- Test hardware reali dopo ogni step
- Nessuna ottimizzazione prematura
