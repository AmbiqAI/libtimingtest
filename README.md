# ns-aot-unit-test

A test harness for HeliosAOT to verify the correctness of the AOT compilation process.

## Notes

To view disassembly of the compiled kernel, use the following command:

```bash
arm-none-eabi-objdump -d ./build/apollo5b_evb/arm-none-eabi/src/nlrls.o
```
