Use the `researcher` subagent to answer the following hardware or protocol question:

**Question:** $ARGUMENTS

The researcher should:
1. Search the local `Docs/` folder first
2. Fetch primary sources (datasheets, reference manuals, NTCIP standards) from the web
3. Reason step by step from the primary source — no guessing
4. Give a precise, quoted answer with register values, timing numbers, OID paths, or AT commands as applicable
5. Explicitly flag anything not confirmed from a primary source

Relevant hardware in this project: STM32H743VIT6 (RM0433), NTCIP 1201 v03, NTCIP 1202 v03, Quectel GPS (L26/L76/L86) and GPRS/LTE (EC21/EC25/M35/BG96) modules, AT24Cxx I2C EEPROM, M95xxx SPI EEPROM, FreeRTOS 10.x, LwIP 2.1.x SNMP.
