# Set output encoding to UTF-8
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8

# Set working dir
Set-Location -Path $PSScriptRoot

# WSL distribution that has ns-3.47 installed (see: make setup / scripts/install_wsl.sh)
$distro = "Ubuntu"

# Prompt user to select a .cc file
Add-Type -AssemblyName System.Windows.Forms
$OpenFileDialog = New-Object System.Windows.Forms.OpenFileDialog
$OpenFileDialog.InitialDirectory = (Get-Location).Path
$OpenFileDialog.Filter = "C++ Files (*.cc)|*.cc"
$OpenFileDialog.Multiselect = $false

Write-Host "> Choose the file to run."

if ($OpenFileDialog.ShowDialog() -eq [System.Windows.Forms.DialogResult]::OK) {
    $selectedFile = $OpenFileDialog.FileName
    $fileName = $OpenFileDialog.SafeFileName
    Write-Host "Selected file: $selectedFile"
} else {
    Write-Host "No file selected. Exiting script."
    exit
}

# Prompt the user for any arguments
Write-Host ""
$execArgs = Read-Host "Execution Arguments (e.g. --distance=100)"
Write-Host ""

# Inform the user what will be executed
Write-Host "-> Building and running" -NoNewline
Write-Host " $fileName " -ForegroundColor Cyan -NoNewline
Write-Host "with arguments:" -NoNewline
Write-Host " $execArgs" -ForegroundColor Yellow
Write-Host ""

# Resolve the repo root and the selected file as WSL paths
$repo = $PSScriptRoot
$relative = [IO.Path]::GetRelativePath($repo, $selectedFile).Replace([char]92, [char]47)
$repoForWsl = $repo.Replace([char]92, [char]47)
$wslRepo = (wsl -d $distro -- wslpath -a $repoForWsl).Trim()
if ($LASTEXITCODE -ne 0) {
    Write-Host "Could not reach the '$distro' WSL distribution. Run 'make setup' inside it first (see docs/environment.md)." -ForegroundColor Red
    exit $LASTEXITCODE
}

$splitArgs = @()
if ($execArgs.Trim().Length -gt 0) {
    $splitArgs = $execArgs -split '\s+'
}

Write-Host "--> Running inside WSL ('$distro'):"
Write-Host "`t scripts/run_cpp.sh $relative $execArgs" -ForegroundColor Green
Write-Host ""

wsl -d $distro -- bash "$wslRepo/scripts/run_cpp.sh" "$relative" @splitArgs
$exitCode = $LASTEXITCODE

Write-Host ""
if ($exitCode -eq 0) {
    Write-Host "==> Done."
} else {
    Write-Host "==> Failed (exit code $exitCode)." -ForegroundColor Red
}
Read-Host "Press Enter to stop..."
