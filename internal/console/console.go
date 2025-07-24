package console

import (
	"fmt"
	"os"
)

// Color codes for terminal output
const (
	Reset  = "\033[0m"
	Red    = "\033[31m"
	Green  = "\033[32m"
	Yellow = "\033[33m"
	Blue   = "\033[34m"
	Purple = "\033[35m"
	Cyan   = "\033[36m"
	White  = "\033[37m"
	Bold   = "\033[1m"
)

// PrintError prints a styled error message
func PrintError(line int, message string) {
	fmt.Printf("%s[⛔ ERROR]%s @ line %d :: %s\n", Red+Bold, Reset, line, message)
}

// PrintSuccess prints a styled success message
func PrintSuccess(line int, message string) {
	fmt.Printf("%s[✅ DONE]%s @ line %d :: %s\n", Green+Bold, Reset, line, message)
}

// PrintWarning prints a styled warning message
func PrintWarning(line int, message string) {
	fmt.Printf("%s[⚠️ WARNING]%s @ line %d :: %s\n", Yellow+Bold, Reset, line, message)
}

// PrintInfo prints a simple info message (simplified)
func PrintInfo(message string) {
	// Removed decorative output - keep it simple
}

// PrintHeader prints a simple header (simplified)
func PrintHeader(filename string) {
	// Removed decorative output - keep it simple
}

// PrintCompilerStats prints compilation statistics
func PrintCompilerStats(imports, variables, constants int) {
	fmt.Printf("%s📊 Compilation Stats:%s\n", Purple+Bold, Reset)
	fmt.Printf("   📦 Imports: %d\n", imports)
	fmt.Printf("   🔄 Variables: %d\n", variables)
	fmt.Printf("   📌 Constants: %d\n", constants)
	fmt.Println("=====================================")
}

// IsTerminal checks if output is to a terminal (for color support)
func IsTerminal() bool {
	fileInfo, _ := os.Stdout.Stat()
	return (fileInfo.Mode() & os.ModeCharDevice) != 0
}
