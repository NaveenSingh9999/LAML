package evaluator

import (
	"fmt"
	"regexp"
	"strconv"
	"strings"
)

// FormatSpecifier represents a C-style format specifier
type FormatSpecifier struct {
	Type      string // s, d, f, c, b
	Precision *int   // for %.2f style
	Position  int    // position in the format string
}

// ParseFormatString extracts format specifiers from a string
func ParseFormatString(format string) ([]FormatSpecifier, error) {
	var specifiers []FormatSpecifier
	re := regexp.MustCompile(`%(?:\.(\d+))?([sdfcb])`)
	matches := re.FindAllStringSubmatch(format, -1)

	for _, match := range matches {
		spec := FormatSpecifier{
			Type: match[2],
		}

		// Handle precision for floats
		if match[1] != "" {
			if precision, err := strconv.Atoi(match[1]); err == nil {
				spec.Precision = &precision
			}
		}

		specifiers = append(specifiers, spec)
	}

	return specifiers, nil
}

// ProcessEscapeSequences handles C-style escape sequences in strings
func ProcessEscapeSequences(input string) string {
	result := strings.Builder{}

	for i := 0; i < len(input); i++ {
		if input[i] == '\\' && i+1 < len(input) {
			switch input[i+1] {
			case 'n':
				result.WriteByte('\n')
				i++ // skip next character
			case 't':
				result.WriteByte('\t')
				i++
			case 'r':
				result.WriteByte('\r')
				i++
			case 'b':
				result.WriteByte('\b')
				i++
			case 'a':
				result.WriteByte('\a')
				i++
			case '\\':
				result.WriteByte('\\')
				i++
			case '\'':
				result.WriteByte('\'')
				i++
			case '"':
				result.WriteByte('"')
				i++
			default:
				// If not a recognized escape sequence, keep the backslash
				result.WriteByte(input[i])
			}
		} else {
			result.WriteByte(input[i])
		}
	}

	return result.String()
}

// FormatWithSpecifiers applies C-style format specifiers to a string with arguments
func FormatWithSpecifiers(format string, args []interface{}) (string, error) {
	// First process escape sequences
	processedFormat := ProcessEscapeSequences(format)

	// Parse format specifiers
	specifiers, err := ParseFormatString(processedFormat)
	if err != nil {
		return "", err
	}

	// Validate argument count
	if len(specifiers) != len(args) {
		return "", fmt.Errorf("format specifier count (%d) does not match argument count (%d)",
			len(specifiers), len(args))
	}

	// Apply format specifiers
	result := processedFormat
	for i := len(specifiers) - 1; i >= 0; i-- {
		spec := specifiers[i]
		arg := args[i]

		var formatted string
		switch spec.Type {
		case "s":
			formatted = fmt.Sprintf("%v", arg)
		case "d":
			if val, ok := convertToInt(arg); ok {
				formatted = fmt.Sprintf("%d", val)
			} else {
				return "", fmt.Errorf("argument %d cannot be formatted as integer", i+1)
			}
		case "f":
			if val, ok := convertToFloat(arg); ok {
				if spec.Precision != nil {
					formatted = fmt.Sprintf("%."+strconv.Itoa(*spec.Precision)+"f", val)
				} else {
					formatted = fmt.Sprintf("%.6f", val) // default 6 decimal places
				}
			} else {
				return "", fmt.Errorf("argument %d cannot be formatted as float", i+1)
			}
		case "c":
			if val, ok := convertToChar(arg); ok {
				formatted = string(val)
			} else {
				return "", fmt.Errorf("argument %d cannot be formatted as character", i+1)
			}
		case "b":
			if val, ok := convertToBool(arg); ok {
				formatted = fmt.Sprintf("%t", val)
			} else {
				return "", fmt.Errorf("argument %d cannot be formatted as boolean", i+1)
			}
		default:
			return "", fmt.Errorf("unsupported format specifier: %s", spec.Type)
		}

		// Replace the format specifier with the formatted value
		// Use a simpler replacement approach
		if spec.Precision != nil {
			pattern := fmt.Sprintf("%%.%d%s", *spec.Precision, spec.Type)
			result = strings.Replace(result, pattern, formatted, 1)
		} else {
			pattern := fmt.Sprintf("%%%s", spec.Type)
			result = strings.Replace(result, pattern, formatted, 1)
		}
	}

	return result, nil
}

// Helper functions for type conversion
func convertToInt(value interface{}) (int64, bool) {
	switch v := value.(type) {
	case int:
		return int64(v), true
	case int64:
		return v, true
	case float32:
		return int64(v), true
	case float64:
		return int64(v), true
	case string:
		if val, err := strconv.ParseInt(v, 10, 64); err == nil {
			return val, true
		}
	}
	return 0, false
}

func convertToFloat(value interface{}) (float64, bool) {
	switch v := value.(type) {
	case int:
		return float64(v), true
	case int64:
		return float64(v), true
	case float32:
		return float64(v), true
	case float64:
		return v, true
	case string:
		if val, err := strconv.ParseFloat(v, 64); err == nil {
			return val, true
		}
	}
	return 0, false
}

func convertToChar(value interface{}) (rune, bool) {
	switch v := value.(type) {
	case int32: // rune is an alias for int32, so handle both cases here
		return rune(v), true
	case int:
		return rune(v), true
	case string:
		if len(v) == 1 {
			for _, r := range v {
				return r, true
			}
		}
		if len(v) > 0 && v[0] == '\'' && len(v) == 3 && v[2] == '\'' {
			return rune(v[1]), true
		}
	}
	return 0, false
}

func convertToBool(value interface{}) (bool, bool) {
	switch v := value.(type) {
	case bool:
		return v, true
	case int:
		return v != 0, true
	case int64:
		return v != 0, true
	case float64:
		return v != 0.0, true
	case string:
		if val, err := strconv.ParseBool(v); err == nil {
			return val, true
		}
		return strings.ToLower(v) == "true", true
	}
	return false, false
}
