set OUT_DIR=C:\Users\ORA\STM32CubeIDE\workspace_1.5.1\UBA_6_Rev2\Proto
set NANOPB_GEN=C:\Users\ORA\Downloads\Work\nanopb-0.4.8-windows-x86\generator\nanopb_generator.py

python "%NANOPB_GEN%" UBA_Battery.proto -D "%OUT_DIR%"
python "%NANOPB_GEN%" UBA_Message.proto -D "%OUT_DIR%"
python "%NANOPB_GEN%" UBA_PROTO_BPT.proto -D "%OUT_DIR%"
python "%NANOPB_GEN%" UBA_PROTO_CALIBRATION.proto -D "%OUT_DIR%"
python "%NANOPB_GEN%" UBA_PROTO_CHANNEL.proto -D "%OUT_DIR%"
python "%NANOPB_GEN%" UBA_PROTO_CMD.proto -D "%OUT_DIR%"
python "%NANOPB_GEN%" UBA_PROTO_LINE.proto -D "%OUT_DIR%"
python "%NANOPB_GEN%" UBA_PROTO_QUERY.proto -D "%OUT_DIR%"
python "%NANOPB_GEN%" UBA_PROTO_TR.proto -D "%OUT_DIR%"
python "%NANOPB_GEN%" UBA_PROTO_DATA_LOG.proto -D "%OUT_DIR%"
python "%NANOPB_GEN%" UBA_PROTO_UBA6.proto -D "%OUT_DIR%"
python "%NANOPB_GEN%" UBA_PROTO_FM.proto -D "%OUT_DIR%"
