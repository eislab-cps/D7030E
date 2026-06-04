# Set output encoding to UTF-8
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8

# Set working dir
Set-Location -Path $PSScriptRoot

# Check if the Docker container 'ns3dev' is running
$containerName = "ns3dev"
$running = docker ps --filter "name=$containerName" --filter "status=running" --format "{{.Names}}"

if (-not $running) {
    Write-Host "Container '$containerName' is not running. Starting it..."
    docker compose up -d
    # Wait a few seconds to ensure container is fully up
    # Start-Sleep -Seconds 3
    Write-Host "Container '$containerName' is now running."
}

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

    # Ensure ./exec/ exists
    $execDir = Join-Path (Get-Location) "exec"
    if (-not (Test-Path $execDir)) {
        New-Item -ItemType Directory -Path $execDir | Out-Null
    }

    # Clear all .cc files in ./exec/
    Get-ChildItem -Path $execDir -Filter "*.cc" -File | Remove-Item -Force
    Write-Host "Cleared existing .cc files in $execDir"

    # Copy the selected file into ./exec/
    Copy-Item -Path $selectedFile -Destination $execDir -Force
    Write-Host "Copied $selectedFile to $execDir"
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

$cmd = 'cd /work && source scripts/setup_env.sh && cd "$NS3_DIR" && ns3 build && ns3 run exec -- ' + $execArgs

Write-Host "--> Running command inside Docker container '$containerName':"
Write-Host "`t $cmd" -ForegroundColor Green
Write-Host ""

# Start an interactive bash session in the Docker container

docker exec -it $containerName bash -c $cmd

Write-Host ""
Write-Host "==> Done."
Read-Host "Press Enter to stop..."