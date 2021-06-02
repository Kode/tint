// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package gen

import (
	"fmt"
	"io"
	"reflect"
	"strings"
	"text/template"
	"unicode"

	"dawn.googlesource.com/tint/tools/src/cmd/intrinsic-gen/sem"
)

type generator struct {
	s      *sem.Sem
	cached struct {
		intrinsicTable *IntrinsicTable // lazily built by intrinsicTable()
	}
}

// Generate executes the template tmpl using the provided semantic
// information, writing the output to w.
// See https://golang.org/pkg/text/template/ for documentation on the template
// syntax.
func Generate(s *sem.Sem, tmpl string, w io.Writer) error {
	g := generator{s: s}
	return g.generate(tmpl, w)
}

func (g *generator) generate(tmpl string, w io.Writer) error {
	t, err := template.
		New("<template>").
		Funcs(map[string]interface{}{
			"Map":                   newMap,
			"Iterate":               iterate,
			"Title":                 strings.Title,
			"PascalCase":            pascalCase,
			"SplitDisplayName":      splitDisplayName,
			"IsTemplateTypeParam":   is(&sem.TemplateTypeParam{}),
			"IsTemplateNumberParam": is(&sem.TemplateNumberParam{}),
			"IsTemplateEnumParam":   is(&sem.TemplateEnumParam{}),
			"IsFirstIn":             isFirstIn,
			"IsLastIn":              isLastIn,
			"IntrinsicTable":        g.intrinsicTable,
		}).
		Option("missingkey=error").
		Parse(tmpl)
	if err != nil {
		return err
	}
	return t.Execute(w, map[string]interface{}{
		"Sem": g.s,
	})
}

// intrinsicTable lazily calls and returns the result of buildIntrinsicTable(),
// caching the result for repeated calls.
func (g *generator) intrinsicTable() (*IntrinsicTable, error) {
	if g.cached.intrinsicTable == nil {
		var err error
		g.cached.intrinsicTable, err = buildIntrinsicTable(g.s)
		if err != nil {
			return nil, err
		}
	}
	return g.cached.intrinsicTable, nil
}

// Map is a simple generic key-value map, which can be used in the template
type Map map[interface{}]interface{}

func newMap() Map { return Map{} }

// Put adds the key-value pair into the map.
// Put always returns an empty string so nothing is printed in the template.
func (m Map) Put(key, value interface{}) string {
	m[key] = value
	return ""
}

// Get looks up and returns the value with the given key. If the map does not
// contain the given key, then nil is returned.
func (m Map) Get(key interface{}) interface{} {
	return m[key]
}

// is returns a function that returns true if the value passed to the function
// matches the type of 'ty'.
func is(ty interface{}) func(interface{}) bool {
	rty := reflect.TypeOf(ty)
	return func(v interface{}) bool {
		return reflect.TypeOf(v) == rty
	}
}

// isFirstIn returns true if v is the first element of the given slice.
func isFirstIn(v, slice interface{}) bool {
	s := reflect.ValueOf(slice)
	count := s.Len()
	if count == 0 {
		return false
	}
	return s.Index(0).Interface() == v
}

// isFirstIn returns true if v is the last element of the given slice.
func isLastIn(v, slice interface{}) bool {
	s := reflect.ValueOf(slice)
	count := s.Len()
	if count == 0 {
		return false
	}
	return s.Index(count-1).Interface() == v
}

// iterate returns a slice of length 'n', with each element equal to its index.
// Useful for: {{- range Iterate $n -}}<this will be looped $n times>{{end}}
func iterate(n int) []int {
	out := make([]int, n)
	for i := range out {
		out[i] = i
	}
	return out
}

// pascalCase returns the snake-case string s transformed into 'PascalCase',
// Rules:
// * The first letter of the string is capitalized
// * Characters following an underscore or number are capitalized
// * Underscores are removed from the returned string
// See: https://en.wikipedia.org/wiki/Camel_case
func pascalCase(s string) string {
	b := strings.Builder{}
	upper := true
	for _, r := range s {
		if r == '_' {
			upper = true
			continue
		}
		if upper {
			b.WriteRune(unicode.ToUpper(r))
			upper = false
		} else {
			b.WriteRune(r)
		}
		if unicode.IsNumber(r) {
			upper = true
		}
	}
	return b.String()
}

// splitDisplayName splits displayName into parts, where text wrapped in {}
// braces are not quoted and the rest is quoted. This is used to help process
// the string value of the [[display()]] decoration. For example:
//   splitDisplayName("vec{N}<{T}>")
// would return the strings:
//   [`"vec"`, `N`, `"<"`, `T`, `">"`]
func splitDisplayName(displayName string) []string {
	parts := []string{}
	pending := strings.Builder{}
	for _, r := range displayName {
		switch r {
		case '{':
			if pending.Len() > 0 {
				parts = append(parts, fmt.Sprintf(`"%v"`, pending.String()))
				pending.Reset()
			}
		case '}':
			if pending.Len() > 0 {
				parts = append(parts, pending.String())
				pending.Reset()
			}
		default:
			pending.WriteRune(r)
		}
	}
	if pending.Len() > 0 {
		parts = append(parts, fmt.Sprintf(`"%v"`, pending.String()))
	}
	return parts
}