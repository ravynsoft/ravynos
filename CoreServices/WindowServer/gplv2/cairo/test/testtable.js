/* configuration */
/* TODO: UNTESTED count can't be shown because it's not tracked explicitly */
headerResults = [ "PASS", "NEW", "FAIL", "XFAIL", "CRASHED" ];
logResults    = [ "PASS", "NEW", "FAIL", "XFAIL", "CRASH!" ];
resultToImgs = {
    "PASS"     : [],
    "NEW"      : [ "output" ],
    "FAIL"     : [ "output", "difference", "reference" ],
    "XFAIL"    : [],
    "UNTESTED" : [],
    "CRASHED"  : []
};

resultToString = {
    "PASS"     : "",
    "NEW"      : "",
    "FAIL"     : "",
    "XFAIL"    : "",
    "UNTESTED" : "",
    "CRASHED"  : "CRASHED!"
};

resultField = "result";
rowFields = [ "test", "offset", "scale", "similar" ];
colFields = [ "target", "format" ];
allFields = [ resultField ].concat (rowFields, colFields);


/* globals: */
function resetGlobals () {
    dragElement = undefined;
    table = document.getElementById ("testTable");
    while (table.rows.length)
	table.deleteRow (0);
    colsArray = [ "HrowHeader" ];
    colsMap = undefined;
    headerId = "HcolHeader";

    fTrue = function (x) { return true; };

    empty = new Row ();
    header = new Row ();
    header[colsArray[0]].toString = function () { return ""; };

    untested = new Test ();
    untested[resultField] = "UNTESTED";
}


/* utility functions */
function isKey (key) { return key[key.length-1] == ':'; }
function normalizeKey (key) { return key.toLowerCase ().replace (/[^a-z0-9]/, ""); }
function isVisible (x) { return x.style.display != "none"; }

function link (html, url) { return "<a href='" + url + "'>" + html + "</a>"; }
function image (url) { return "<img src='" + url + "'>"; }
function span (html, id, cls) { return "<span id='" + id + "' class='" + cls + "' onmousedown='startDrag (event)' onmouseup='mouseUp (event)'>" + html + "</span>"; }

function fieldsToHTML (bColumns, values) {
    var fields = bColumns ? colFields : rowFields;
    var prefix = bColumns ? "c" : "r";
    var tmpRE = arrayApply (function (x) { return "[^/]*"; }, fields);
    var r = Array ();
    for (var i = 0; i < fields.length; i++)
	if (fields[i] == "test") {
	    r.push (link (values[fields[i]], "output/" + values[fields[i]] + ".log"));
	} else {
	    tmpRE[i] = values[fields[i]];
	    r.push (span (values[fields[i]], prefix + "/" + tmpRE.join ("/") + "/", fields[i]));
	    tmpRE[i] = "[^/]*";
	}
    return r.join ("/");
}

function inArray (value, array) {
    for (var i = 0; i < array.length; i++)
	if (value == array[i])
	    return true;
    return false;
}

function arrayApply (fun, array) {
    var r = new Array ();
    for (var i = 0; i < array.length; i++)
	r.push (fun(array[i]));
    return r;
}

function arrayPred (pred, array) {
    var r = new Array ();
    for (var i = 0; i < array.length; i++)
	if (pred (array[i]))
	    r.push (array[i]);
    return r;
}

function arrayMap (map, array) { return arrayApply (function (x) { return map[x]; }, array); }

function binSearch (rows, newId){
    var min = 0;
    var max = rows.length;

    while (max - min > 1) {
	var mid = (max + min) >> 1;
        if (rows[mid].id > newId)
	    max = mid;
        else
	    min = mid;
    }

    if (max == min)
	return max;
    else
	return rows[min].id > newId ? min : max;
}

/* dynamic table utils */
function updateCurrent () {
    for (var i = 0; i < table.rows.length; i++) {
	var row = table.rows[i];
	if (isVisible (row)) {
	    /* j starts from 1 because we want to ignore _rowHeader */
	    for (var j = 1; j < row.cells.length; j++)
		if (row.id[0] == "H")
		    for (var k = 0; k < headerResults.length; k++)
			header[row.cells[j].id].current[headerResults[k]] = 0;
		else if (isVisible (row.cells[j]))
		    header[row.cells[j].id].current[row.cells[j].className]++;
	}
    }

    updateHeader ();
}

function setVisible (array, subsetPred, visibilityPred, visibleFlag) {
    var modified = false, somethingVisible = false;
    for (var i = 0; i < array.length; i++)
	if (array[i].id[0] != "H") {
	    if (subsetPred (array[i])) {
		var wanted = visibilityPred (array[i]);
		if (isVisible (array[i]) != wanted) {
		    modified = true;
		    array[i].style.display = wanted ? visibleFlag : "none";
		}
	    }
	    somethingVisible = somethingVisible || isVisible (array[i]);
	}
    return modified && somethingVisible;
}

function setVisibleOnly (array, pred, visibleFlag) {
    return setVisible (array, fTrue, pred, visibleFlag);
}

function flipVisible (array, subsetPred, visibleFlag) {
    return setVisible (array, subsetPred, function (x) { return !isVisible (x); }, visibleFlag);
}


/* event handling */
function ignoreEvent (event) {
    if (event.preventDefault)
        event.preventDefault();
    else
        event.returnValue= false;
    return false;
}

function mouseUp (event) {
    var visFun;
    if (event.button == 0)
	visFun = setVisibleOnly;
    else if (event.button == 2)
	visFun = flipVisible;
    else
	return false;

    var structureFun;
    if (event.target.id[0] == "r") /* rows */
	structureFun = function (f, p) { return f (table.rows, p, "table-row"); };
    else if (event.target.id[0] == "c") /* cols */
	structureFun = function (f, p) { return inArray (true, arrayApply (function (row) { return f (row.cells, p, "table-cell") }, table.rows)) };
    else
	return false;

    var pred;
    if (event.target.id[1] == "/") { /* regexp */
	var re = new RegExp (event.target.id);
	pred = function (x) { return re.test (x.id); };
    } else if (event.target.id[1] == "#") { /* counters */
	var s = event.target.id.substr (2).split ("/");
	pred = function (row) { return row.cells[s[0]].className == s[1]; }
    } else
	return false;

    if (!structureFun (visFun, pred))
	if (!structureFun (flipVisible, fTrue))
	    structureFun (flipVisible, fTrue);

    updateCurrent ();

    return false;
}

function noDrag (event) {
    dragElement = undefined;
    return false;
}

function startDrag (event) {
    if (event.button == 0)
	dragElement = event.target;
    else
	dragElement = undefined;
    return false;
}

function endDrag (event) {
    if (!dragElement)
	return false;

    if (event.currentTarget.id == colsArray[0] &&
	inArray (dragElement.className, colFields)) {
	rowFields.push (dragElement.className);
	colFields = arrayPred (function (x) { return x != dragElement.className; }, colFields);
    } else if (event.currentTarget.id == headerId &&
	       inArray (dragElement.className, rowFields)) {
	colFields.push (dragElement.className);
	rowFields = arrayPred (function (x) { return x != dragElement.className; }, rowFields);
    } else
	return true;

    reloadAll ();
    return false;
}


/* table content */
function Row (id, t) {
    this[colsArray[0]] = new RowHeader (id, t);

    this.get = function (c) { return this[c] != undefined ? this[c] : untested; }
    this.getHTML = function (c) { return this.get(c).toString (); };
    this.setStyle = function (c, element) { return this.get(c).setStyle (element); };
}

function ColumnHeader (id, values) {
    this.id = id;
    this.values = values;
    this.total = new Object ();
    this.current = new Object ();

    for (var i = 0; i < headerResults.length; i++) {
	this.total[headerResults[i]] = 0;
	this.current[headerResults[i]] = 0;
    }

    this.toString = function () {
	var counts = new Array ();
	for (var i = 0; i < headerResults.length; i++) {
	    var hr = headerResults[i];
	    var s = span (this.current[hr], "r#" + colsMap[this.id] + "/" + hr, hr);
	    if (this.current[hr] != this.total[hr])
		s += span ("[" + this.total[hr] + "]", "r#" + colsMap[this.id] + "/" + hr, hr);
	    counts.push (s);
	}

	return fieldsToHTML (true, this.values) + "<br>" + counts.join ("/");
    }

    this.setStyle = function (element) { };
}

function RowHeader (id, values) {
    this.id = id;
    this.values = values;
    this.toString = function () { return fieldsToHTML (false, this.values); }
    this.setStyle = function (element) { element.onmouseup = endDrag; };
}

function Test () {
    this.rowId = function () { return "r/" + arrayMap (this, rowFields).join("/") + "/"; };
    this.colId = function () { return "c/" + arrayMap (this, colFields).join("/") + "/"; };
    this.isComplete = function () { return !inArray (undefined, arrayMap (this, allFields)); }
    this.toString = function () {
	var images = arrayMap (this, resultToImgs[this[resultField]]);
	images = arrayPred (function (x) { return x != undefined; }, images);
	images = arrayApply (function (x) { return link (image (x), x); }, images);
	images.push (resultToString[this[resultField]]);
	return images.join (" ");
    };

    this.setStyle = function (element) { element.className = this[resultField]; };

    this.addData = function (array) {
	for (var i = 0; i < array.length - 1; i += 2)
	    if (isKey (array[i]))
		this[normalizeKey (array[i])] = array[i+1];
    };
}


/* table creation */
function insertCell (domRow, nid, tests) {
    var domCell = domRow.insertCell (nid);
    domCell.id = colsArray[nid];
    domCell.innerHTML = tests.getHTML (colsArray[nid]);
    tests.setStyle (colsArray[nid], domCell);
}

function updateRow (row, tests) {
    var domRow = document.getElementById (row);
    if (!domRow) {
	domRow = table.insertRow (binSearch (table.rows, row));
	domRow.id = row;
    }

    for (var i = 0; i < colsArray.length; i++)
	if (i >= domRow.cells.length || domRow.cells[i].id != colsArray[i])
	    insertCell (domRow, i, tests);
}

function updateHeader () {
    var visibility;
    var domRow = document.getElementById (headerId);
    if (domRow) {
	visibility = new Object ();
	for (var i = 0; i < domRow.cells.length; i++)
	    visibility[domRow.cells[i].id] = domRow.cells[i].style.display;
	table.deleteRow (domRow.rowIndex);
    }

    updateRow (headerId, header);
    table.rows[0].onmouseup = endDrag;

    if (visibility)
	for (var i = 0; i < colsArray.length; i++)
	    if (visibility[colsArray[i]])
		table.rows[0].cells[colsMap[colsArray[i]]].style.display = visibility[colsArray[i]];
}

function updateTable () {
    colsArray.sort ();

    colsMap = new Object ();
    for (var i = 0; i < colsArray.length; i++)
	colsMap[colsArray[i]] = i;

    updateHeader ();
    for (var i = 0; i < table.rows.length; i++)
	updateRow (table.rows[i].id, empty);
}


/* log file parsing */
function parseTest (testData) {
    var colsChanged = false;
    var rows = new Array ();
    var data = new Object ();
    var t = new Test ();
    var lines = testData.replace (/\r/g, "").split ("\n");
    for (var i = 0; i < lines.length; i++) {
	t.addData (lines[i].split (" "));
	if (t.isComplete ()) {
	    var c = t.colId ();
	    if (header[c] == undefined) {
		colsArray.push (c);
		header[c] = new ColumnHeader (c, t);
		colsChanged = true;
	    }

	    var r = t.rowId ();
	    if (!data[r]) {
		rows.push (r);
		data[r] = new Row (r, t);
	    }

	    data[r][c] = t;
	    header[c].total[t[resultField]]++;
	    header[c].current[t[resultField]]++;
	    t = new Test ();
	}
    }

    if (colsChanged)
	updateTable ();
    else
	updateHeader ();

    for (var i = 0; i < rows.length; i++)
	updateRow (rows[i], data[rows[i]]);
}

function parseFile (fileName, parser) {
    var req = new XMLHttpRequest ();
    req.onreadystatechange = function () {
	if (req.readyState == 4)
	    parser (req.responseText);
    }

    try {
	req.open ("GET", fileName);
	req.send (null);
    } catch (e) {}
} 

function parseTestList (listData) {
    var summaryRE = /\d+ Passed, \d+ Failed \x5b\d+ crashed, \d+ expected\x5d, \d+ Skipped/;
    var lines = listData.replace (/\r/g, "").split ("\n");
    for (var i = 0; i < lines.length; i++) {
	if (summaryRE.test (lines[i]))
	    return;
	
	var words = lines[i].split (" ");
	if (words.length >= 2 &&
	    words[0][words[0].length-1] == ":" &&
	    inArray (words[1], logResults))
	    parseFile ("output/" + words[0].substr (0, words[0].length-1) + ".log", parseTest);
    }
}

function reloadAll() {
    resetGlobals ();

    parseFile ("cairo-test-suite.log", parseTestList);
} 

window.onload = reloadAll;
