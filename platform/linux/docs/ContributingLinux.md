##Contributing.

Thank you for contributing to Solar2DTux! Before you get started, please review this document to get an understanding of our guidelines.

**Code Style**

Please ensure you follow the below settings for Codelite when contributing, as the coding style must be adhered to, or any pull requests not adhering to this style will be rejected. This may seem harsh, but we're aiming for consistency, so it's important that you follow the guidelines.

We use camel case strictly in this project.

**Indentation**

Found in `Settings > Preferances > Indentation`. 

* Use tabs in indentation: Yes.
* Columns per indentaiton level: 4.
* Columns per tab character in document: 4.

Note: Make sure when you save a file `ctrl + s` that whitespace characters are removed. This simple test will allow you to know if formatting on save is enabled.

**Whitespace**

Found in `Settings > Preferances > Guides`.

* Whitespace visibility: Visible always.
* EOL mode: default.
* Line spacing: 0.

**Formatting**

We use Codelite's `source code formatter` plugin with the following settings:

Style Settings:

(General):

* C++ formatter: AStyle.

(C++):

* PreDefined Styles: ANSI.
* Brackets: Break closing.
* Identation: Switches, Namespaces, Labels.
* Formatting: Pad Operators, One Line Keep Blocks.

**Pointer/General Placement**

Good:

`wxString *myString;`

Bad:

`wxString* myString;`

Good:

`void doSomething(wxString &myString)`

Bad:

`void doSomething(wxString& myString)`
