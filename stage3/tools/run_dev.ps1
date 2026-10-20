# Stage-3 development run (harness check only). Run from the project root.
$env:PATH = "C:\msys64\usr\bin;" + $env:PATH
python -I stage3/string3/run_string3.py --manifest dev *> stage3/results/s3_dev_console.txt
"exit=$LASTEXITCODE" | Add-Content stage3/results/s3_dev_console.txt
