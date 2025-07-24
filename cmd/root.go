package cmd

import (
	"fmt"
	"os"
	"path/filepath"

	"laml/internal/compiler"
	"laml/internal/console"

	"github.com/spf13/cobra"
)

var rootCmd = &cobra.Command{
	Use:   "laml",
	Short: "LAML - Low Abstraction Machine Language Compiler",
	Long: `LAML is a low-abstraction, sentence-like general-purpose compiled language.
Built for speed, flexibility, and direct kernel interaction.`,
}

var compileCmd = &cobra.Command{
	Use:   "compile [file.lm]",
	Short: "Compile a LAML source file",
	Args:  cobra.ExactArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
		sourceFile := args[0]

		// Print header
		console.PrintHeader(sourceFile)

		// Check if file exists and has .lm extension
		if filepath.Ext(sourceFile) != ".lm" {
			console.PrintError(0, "File must have .lm extension")
			os.Exit(1)
		}

		if _, err := os.Stat(sourceFile); os.IsNotExist(err) {
			console.PrintError(0, fmt.Sprintf("File %s does not exist", sourceFile))
			os.Exit(1)
		}

		// Initialize compiler
		c := compiler.New()

		// Compile the source file
		if err := c.CompileFile(sourceFile); err != nil {
			console.PrintError(0, fmt.Sprintf("Compilation failed: %v", err))
			os.Exit(1)
		}

		console.PrintSuccess(0, fmt.Sprintf("Successfully compiled %s", sourceFile))
	},
}

var runCmd = &cobra.Command{
	Use:   "run [file.lm]",
	Short: "Compile and run a LAML source file",
	Args:  cobra.ExactArgs(1),
	Run: func(cmd *cobra.Command, args []string) {
		sourceFile := args[0]

		// Print header
		console.PrintHeader(sourceFile)

		// Check if file exists and has .lm extension
		if filepath.Ext(sourceFile) != ".lm" {
			console.PrintError(0, "File must have .lm extension")
			os.Exit(1)
		}

		if _, err := os.Stat(sourceFile); os.IsNotExist(err) {
			console.PrintError(0, fmt.Sprintf("File %s does not exist", sourceFile))
			os.Exit(1)
		}

		console.PrintInfo("Compiling and executing...")

		// Initialize compiler
		c := compiler.New()

		// Compile and run the source file
		if err := c.CompileAndRun(sourceFile); err != nil {
			console.PrintError(0, fmt.Sprintf("Execution failed: %v", err))
			os.Exit(1)
		}
	},
}

var versionCmd = &cobra.Command{
	Use:   "version",
	Short: "Print LAML version",
	Run: func(cmd *cobra.Command, args []string) {
		console.PrintInfo("LAML v3.0.0 - Low Abstraction Machine Language")
		console.PrintInfo("Phase 3: Enhanced Developer Experience")
	},
}

func init() {
	rootCmd.AddCommand(compileCmd)
	rootCmd.AddCommand(runCmd)
	rootCmd.AddCommand(versionCmd)
}

func Execute() error {
	return rootCmd.Execute()
}
