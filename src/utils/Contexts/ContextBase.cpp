/*
 * ContextBase.cpp
 *
 *  Created on: Feb 12, 2013
 *      Author: nek3d
 */

#include "ContextBase.h"
#include <unistd.h>
#include <sys/types.h>
#include <cctype>

ContextBase::ContextBase()
:
  _program(UNSPECIFIED_PROGRAM),
  _allFilesOpened(false),
  _genomeFile(NULL),
  _outputFileType(FileRecordTypeChecker::UNKNOWN_FILE_TYPE),
  _outputTypeDetermined(false),
  _skipFirstArgs(0),
  _showHelp(false),
  _obeySplits(false),
  _uncompressedBam(false),
  _useBufferedOutput(true),
  _ioBufSize(0),
  _anyHit(false),
  _noHit(false),
  _writeA(false),
  _writeB(false),
  _leftJoin(false),
  _writeCount(false),
  _writeOverlap(false),
  _writeAllOverlap(false),
  _haveFraction(false),
  _overlapFraction(0.0),
  _reciprocal(false),
  _sameStrand(false),
  _diffStrand(false),
  _sortedInput(false),
  _sortOutput(false),
  _reportDBnameTags(false),
  _reportDBfileNames(false),
  _printHeader(false),
  _printable(true),
   _explicitBedOutput(false),
  _queryFileIdx(-1),
  _bamHeaderAndRefIdx(-1),
  _maxNumDatabaseFields(0),
  _useFullBamTags(false),
  _numOutputRecords(0),
  _hasConstantSeed(false),
  _seed(0),
  _forwardOnly(false),
  _reverseOnly(false),
  _nameCheckDisabled(false),
  _hasColumnOpsMethods(false),
  _keyListOps(NULL),
  _desiredStrand(FileRecordMergeMgr::ANY_STRAND),
  _maxDistance(0),
  _useMergedIntervals(false),
  _reportPrecision(-1),
  _allFilesHaveChrInChromNames(UNTESTED),
  _allFileHaveLeadingZeroInChromNames(UNTESTED),
  _nameConventionWarningTripped(false)

{
	_programNames["intersect"] = INTERSECT;
	_programNames["sample"] = SAMPLE;
	_programNames["map"] = MAP;
	_programNames["merge"] = MERGE;
	_programNames["closest"] = CLOSEST;
	_programNames["subtract"] = SUBTRACT;
	_programNames["jaccard"] = JACCARD;

	if (hasColumnOpsMethods()) {
		_keyListOps = new KeyListOps();
	}
}

ContextBase::~ContextBase()
{
	delete _genomeFile;
	_genomeFile = NULL;

	//close all files and delete FRM objects.
	for (int i=0; i < (int)_files.size(); i++) {
		_files[i]->close();
		delete _files[i];
		_files[i] = NULL;
	}
	if (hasColumnOpsMethods()) {
		delete _keyListOps;
		_keyListOps = NULL;
	}

	// if there was a warning about file name conventions,
	// print it again so it's not lost amid large output.
	if (_nameConventionWarningTripped) {
		cerr << _nameConventionWarningMsg << endl;
	}

}

bool ContextBase::determineOutputType() {
	if (_outputTypeDetermined) {
		return true;
	}
	//test whether output should be BED or BAM.
	//If the user explicitly requested BED, then it's BED.
	if (getExplicitBedOutput()) {
		setOutputFileType(FileRecordTypeChecker::SINGLE_LINE_DELIM_TEXT_FILE_TYPE);
		_outputTypeDetermined = true;
		return true;
	}

	//Otherwise, if the input is BAM, then the output is BAM
	if (getFile(0)->getFileType() == FileRecordTypeChecker::BAM_FILE_TYPE) {
		setOutputFileType(FileRecordTypeChecker::BAM_FILE_TYPE);
		return true;
	}

	//Okay, it's bed.
	setOutputFileType(FileRecordTypeChecker::SINGLE_LINE_DELIM_TEXT_FILE_TYPE);
	_outputTypeDetermined = true;
	return true;


}

void ContextBase::openGenomeFile(const QuickString &genomeFilename)
{
	_genomeFile = new NewGenomeFile(genomeFilename.c_str());
}

void ContextBase::openGenomeFile(const BamTools::RefVector &refVector)
{
	_genomeFile = new NewGenomeFile(refVector);
}

bool ContextBase::parseCmdArgs(int argc, char **argv, int skipFirstArgs) {
	_argc = argc;
	_argv = argv;
	_skipFirstArgs = skipFirstArgs;

	setProgram(_programNames[argv[0]]);

	_argsProcessed.resize(_argc - _skipFirstArgs, false);

	for (_i=_skipFirstArgs; _i < argc; _i++) {
		if (isUsed(_i - _skipFirstArgs)) {
			continue;
		}

		if (strcmp(_argv[_i], "-i") == 0) {
			if (!handle_i()) return false;
		}
		else if (strcmp(_argv[_i], "-g") == 0) {
			if (!handle_g()) return false;
		}
		else if ((strcmp(_argv[_i], "-h") == 0) || (strcmp(_argv[_i], "--help") == 0)) {
			if (!handle_h()) return false;
		}
		else if (strcmp(_argv[_i], "-split") == 0) {
			if (!handle_split()) return false;
		}
        else if (strcmp(_argv[_i], "-bed") == 0) {
			if (!handle_bed()) return false;
       }
        else if (strcmp(_argv[_i], "-ubam") == 0) {
			if (!handle_ubam()) return false;
        }
        else if (strcmp(_argv[_i], "-fbam") == 0) {
			if (!handle_fbam()) return false;
        }
        else if(strcmp(_argv[_i], "-sorted") == 0) {
			if (!handle_sorted()) return false;
        }
        else if (strcmp(_argv[_i], "-nobuf") == 0) {
			if (!handle_nobuf()) return false;
        }
        else if (strcmp(_argv[_i], "-iobuf") == 0) {
			if (!handle_iobuf()) return false;
        }
        else if (strcmp(_argv[_i], "-prec") == 0) {
			if (!handle_prec()) return false;
        }
        else if (strcmp(_argv[_i], "-header") == 0) {
			if (!handle_header()) return false;
        }
        else if (strcmp(_argv[_i], "-n") == 0) {
			if (!handle_n()) return false;
        }
        else if (strcmp(_argv[_i], "-seed") == 0) {
			if (!handle_seed()) return false;
        }
        else if (strcmp(_argv[_i], "-o") == 0) {
			if (!handle_o()) return false;
        }
        else if (strcmp(_argv[_i], "-c") == 0) {
			if (!handle_c()) return false;
        }
        else if (strcmp(_argv[_i], "-null") == 0) {
			if (!handle_null()) return false;
        }
        else if (strcmp(_argv[_i], "-delim") == 0) {
			if (!handle_delim()) return false;
        }
        else if (strcmp(_argv[_i], "-sortout") == 0) {
			if (!handle_sortout()) return false;
        }
        else if (strcmp(_argv[_i], "-nonamecheck") == 0) {
			if (!handle_nonamecheck()) return false;
        }

	}
	return true;
}

bool ContextBase::isValidState()
{
	if (!openFiles()) {
		return false;
	}
	if (!cmdArgsValid()) {
		return false;
	}
	if (!determineOutputType()) {
		return false;
	}
	if (hasColumnOpsMethods()) {

		if (hasIntersectMethods()) {
			for (int i=0; i < (int)_dbFileIdxs.size(); i++) {
				FileRecordMgr *dbFile = getFile(_dbFileIdxs[i]);
				_keyListOps->setDBfileType(dbFile->getFileType());
				if (!_keyListOps->isValidColumnOps(dbFile)) {
					return false;
				}
			}
		} else {
			FileRecordMgr *dbFile = getFile(0);
			_keyListOps->setDBfileType(dbFile->getFileType());
			if (!_keyListOps->isValidColumnOps(dbFile)) {
				return false;
			}
		}
		//if user specified a precision, pass it to
		//keyList ops
		if (_reportPrecision != -1) {
			_keyListOps->setPrecision(_reportPrecision);
		}
	}
	return true;
}


bool ContextBase::cmdArgsValid()
{
	bool retval = true;
	for (_i = _skipFirstArgs; _i < _argc; _i++) {
		if (!isUsed(_i - _skipFirstArgs)) {
			_errorMsg += "\n***** ERROR: Unrecognized parameter: ";
			_errorMsg += _argv[_i];
			_errorMsg += " *****";
			retval = false;
		}
	}
	return retval;
}

bool ContextBase::openFiles() {

	//Make a vector of FileRecordMgr objects by going through the vector
	//of filenames and opening each one.
	if (_allFilesOpened) {
		return true;
	}

	if (_fileNames.size() == 0) {
		//No input was specified. Error and exit.
		_errorMsg += "\n***** ERROR: No input file given. Exiting. *****";
		return false;
	}

	_files.resize(_fileNames.size());

	for (int i = 0; i < (int)_fileNames.size(); i++) {
		FileRecordMgr *frm = getNewFRM(_fileNames[i], i);
		if (hasGenomeFile()) {
			frm->setGenomeFile(_genomeFile);
		}
		//If we're going to do column operations, and an input file
		// is BAM, we'll need the full flags.
		if (hasColumnOpsMethods()) {
			setUseFullBamTags(true);
		}
		frm->setFullBamFlags(_useFullBamTags);
		frm->setIsSorted(_sortedInput);
		frm->setIoBufSize(_ioBufSize);
		if (!frm->open()) {
			return false;
		}
		_files[i] = frm;
	}
	_allFilesOpened = true;
	return true;
}

int ContextBase::getBamHeaderAndRefIdx() {
	if (_bamHeaderAndRefIdx != -1) {
		//already found which BAM file to use for the header
		return _bamHeaderAndRefIdx;
	}
	if (hasIntersectMethods()) {
		if (_files[_queryFileIdx]->getFileType() == FileRecordTypeChecker::BAM_FILE_TYPE) {
			_bamHeaderAndRefIdx = _queryFileIdx;
		} else {
			_bamHeaderAndRefIdx = _dbFileIdxs[0];
		}
		return _bamHeaderAndRefIdx;
	}
	if (_files[0]->getFileType() == FileRecordTypeChecker::BAM_FILE_TYPE) {
		_bamHeaderAndRefIdx = 0;
		return _bamHeaderAndRefIdx;
	}

	return _bamHeaderAndRefIdx;
}

int ContextBase::getUnspecifiedSeed()
{
	// thanks to Rob Long for the tip.
	_seed = (unsigned)time(0)+(unsigned)getpid();
	srand(_seed);
	return _seed;
}

bool ContextBase::handle_bed()
{
	setExplicitBedOutput(true);
	markUsed(_i - _skipFirstArgs);
	return true;
}

bool ContextBase::handle_fbam()
{
	setUseFullBamTags(true);
	markUsed(_i - _skipFirstArgs);
	return true;
}

bool ContextBase::handle_g()
{
	if (_argc <= _i+1) {
		_errorMsg = "\n***** ERROR: -g option given, but no genome file specified. *****";
		return false;
	}
	openGenomeFile(_argv[_i+1]);
	markUsed(_i - _skipFirstArgs);
	_i++;
	markUsed(_i - _skipFirstArgs);
	return true;
}

bool ContextBase::handle_h()
{
	setShowHelp(true);
	markUsed(_i - _skipFirstArgs);
	return true;
}

bool ContextBase::handle_header()
{
	setPrintHeader(true);
	markUsed(_i - _skipFirstArgs);
	return true;
}

bool ContextBase::handle_i()
{
	if (_argc <= _i+1) {
		_errorMsg = "\n***** ERROR: -i option given, but no input file specified. *****";
		return false;
	}
	addInputFile(_argv[_i+1]);
	markUsed(_i - _skipFirstArgs);
	_i++;
	markUsed(_i - _skipFirstArgs);
	return true;
}

bool ContextBase::handle_n()
{
	if (_argc <= _i+1) {
		_errorMsg = "\n***** ERROR: -n option given, but no number of output records specified. *****";
		return false;
	}
	setNumOutputRecords(atoi(_argv[_i + 1]));
	markUsed(_i - _skipFirstArgs);
	_i++;
	markUsed(_i - _skipFirstArgs);
	return true;
}

bool ContextBase::handle_nobuf()
{
	setUseBufferedOutput(false);
	markUsed(_i - _skipFirstArgs);
	return true;
}

bool ContextBase::handle_iobuf()
{
	if (_argc <= _i+1) {
		_errorMsg = "\n***** ERROR: -iobuf option given, but size of input buffer not specified. *****";
		return false;
	}
	if (!parseIoBufSize(_argv[_i + 1])) return false;
	markUsed(_i - _skipFirstArgs);
	_i++;
	markUsed(_i - _skipFirstArgs);
	return true;
}

bool ContextBase::handle_seed()
{
	if (_argc <= _i+1) {
		_errorMsg = "\n***** ERROR: -seed option given, but no seed specified. *****";
		return false;
	}
	_hasConstantSeed = true;
	_seed  = atoi(_argv[_i+1]);
	srand(_seed);
	markUsed(_i - _skipFirstArgs);
	_i++;
	markUsed(_i - _skipFirstArgs);
	return true;
}

bool ContextBase::handle_split()
{
    setObeySplits(true);
    markUsed(_i - _skipFirstArgs);
	return true;
}

bool ContextBase::handle_sorted()
{
	setSortedInput(true);
	markUsed(_i - _skipFirstArgs);
	return true;
}

bool ContextBase::handle_ubam()
{
    setUncompressedBam(true);
    markUsed(_i - _skipFirstArgs);
	return true;
}


// Methods specific to column operations.
// for col ops, -c is the string of columns upon which to operate
bool ContextBase::handle_c()
{
	if (!hasColumnOpsMethods()) {
		return false;
	}
    if ((_i+1) < _argc) {
        _keyListOps->setColumns(_argv[_i + 1]);
        markUsed(_i - _skipFirstArgs);
        _i++;
        markUsed(_i - _skipFirstArgs);
        return true;
    }
    return false;
}


// for col ops, -o is the string of operations to apply to the columns (-c)
bool ContextBase::handle_o()
{
	if (!hasColumnOpsMethods()) {
		return false;
	}
    if ((_i+1) < _argc) {
    	 _keyListOps->setOperations(_argv[_i + 1]);
        markUsed(_i - _skipFirstArgs);
        _i++;
        markUsed(_i - _skipFirstArgs);
        return true;
    }
    return false;
}

bool ContextBase::handle_prec()
{
	if (!hasColumnOpsMethods()) {
		return false;
	}
    if ((_i+1) < _argc) {
    	int prec = atoi(_argv[_i + 1]);
    	if (prec < 1) {
    		_errorMsg += "\n***** ERROR: -prec must be followed by a positive integer. Exiting. *****";
    		return false;
    	}
    	 _reportPrecision = prec;
        markUsed(_i - _skipFirstArgs);
        _i++;
        markUsed(_i - _skipFirstArgs);
        return true;
    }
	_errorMsg += "\n***** ERROR: -prec must be followed by a positive integer. Exiting. *****";
    return false;
}



// for col ops, -null is a NULL value assigned
// when no overlaps are detected.
bool ContextBase::handle_null()
{
	if (!hasColumnOpsMethods()) {
		return false;
	}
    if ((_i+1) < _argc) {
    	 _keyListOps->setNullValue(_argv[_i + 1]);
        markUsed(_i - _skipFirstArgs);
        _i++;
        markUsed(_i - _skipFirstArgs);
        return true;
    }
    return false;
}

//for col ops, delimStr will appear between each item in
//a collapsed but delimited list.
bool ContextBase::handle_delim()
{
	if (!hasColumnOpsMethods()) {
		_errorMsg = "\n***** ERROR: Can't set delimiter for tools without column operations. Exiting. *****";
		return false;
	}
    if ((_i+1) < _argc) {
    	 _keyListOps->setDelimStr(_argv[_i + 1]);
        markUsed(_i - _skipFirstArgs);
        _i++;
        markUsed(_i - _skipFirstArgs);
    }
    return true;
}

bool ContextBase::handle_sortout()
{
	setSortOutput(true);
	markUsed(_i - _skipFirstArgs);
	return true;
}

bool ContextBase::handle_nonamecheck()
{
	setNameCheckDisabled(true);
	markUsed(_i - _skipFirstArgs);
	return true;
}

void ContextBase::setColumnOpsMethods(bool val)
{
	if (val && !_hasColumnOpsMethods) {
		//was off, but we're turning it on.
		_keyListOps = new KeyListOps();
	}
	_hasColumnOpsMethods = val;
}

const QuickString &ContextBase::getColumnOpsVal(RecordKeyVector &keyList) const {
	if (!hasColumnOpsMethods()) {
		return _nullStr;
	}
	return _keyListOps->getOpVals(keyList);
}

FileRecordMgr *ContextBase::getNewFRM(const QuickString &filename, int fileIdx) {

	if (_useMergedIntervals) {
		FileRecordMergeMgr *frm = new FileRecordMergeMgr(filename);
		frm->setStrandType(_desiredStrand);
		frm->setMaxDistance(_maxDistance);
		frm->setFileIdx(fileIdx);
		return frm;
	} else {
		FileRecordMgr *frm = new FileRecordMgr(filename);
		frm->setFileIdx(fileIdx);
		return frm;
	}
}

bool ContextBase::parseIoBufSize(QuickString bufStr)
{
	char lastChar = bufStr[bufStr.size()-1];
	int multiplier = 1;
	if (!isdigit(lastChar)) {
		switch (lastChar) {
		case 'K':
			multiplier = 1 << 10;
			break;
		case 'M':
			multiplier = 1 << 20;
			break;
		case 'G':
			multiplier = 1 << 30;
			break;
		default:
			_errorMsg = "\n***** ERROR: Unrecognized memory buffer size suffix \'";
			_errorMsg += lastChar;
			_errorMsg += "\' given. *****";
			return false;
			break;
		}
		//lop off suffix character
		bufStr.resize(bufStr.size()-1);
	}
	if (!isNumeric(bufStr)) {
		_errorMsg = "\n***** ERROR: argument passed to -iobuf is not numeric. *****";
		return false;
	}
	_ioBufSize = str2chrPos(bufStr) * multiplier;
	if (_ioBufSize < MIN_ALLOWED_BUF_SIZE) {
		_errorMsg = "\n***** ERROR: specified buffer size is too small. *****";
		return false;
	}
	return true;
}

void ContextBase::testNameConventions(const Record *record) {
	if (getNameCheckDisabled() || _nameConventionWarningTripped) return;

	int fileIdx = record->getFileIdx();

	//
	// First test whether chr in chrom names match
	//

	bool hasChr = record->hasChrInChromName();
	testType testChrVal = fileHasChrInChromNames(fileIdx);

	if (testChrVal == UNTESTED) {
		_fileHasChrInChromNames[fileIdx] = hasChr ? YES : NO;
	}
//	else if ((testChrVal == YES && !hasChr) || (testChrVal == NO && hasChr)) {
//		nameConventionWarning(record, _fileNames[fileIdx], " has inconsistent naming convention for record:\n");
//	}
	if ((_allFilesHaveChrInChromNames == YES && !hasChr) || (_allFilesHaveChrInChromNames == NO && hasChr)) {
		nameConventionWarning(record, _fileNames[fileIdx], " has a record where naming convention is inconsistent with other files:\n");
	}

	if (_allFilesHaveChrInChromNames == UNTESTED) {
		_allFilesHaveChrInChromNames = hasChr ? YES : NO;
	}


	//
	// Now test whether leading zero in chrom names match
	//


	bool zeroVal = record->hasLeadingZeroInChromName(hasChr);
	testChrVal = fileHasLeadingZeroInChromNames(fileIdx);
	if (testChrVal == UNTESTED) {
		_fileHasLeadingZeroInChromNames[fileIdx] = zeroVal ? YES : NO;
	}
//	else if ((testChrVal == YES && !zeroVal) || (testChrVal == NO && zeroVal)) {
//		nameConventionWarning(record, _fileNames[fileIdx], " has inconsistent naming convention (leading zero) for record:\n");
//	}
	if ((_allFileHaveLeadingZeroInChromNames == YES && !zeroVal) || (_allFileHaveLeadingZeroInChromNames == NO && zeroVal)) {
		nameConventionWarning(record, _fileNames[fileIdx], " has a record where naming convention (leading zero) is inconsistent with other files:\n");
	}

	if (_allFileHaveLeadingZeroInChromNames == UNTESTED) {
		_allFileHaveLeadingZeroInChromNames = zeroVal ? YES : NO;
	}
}

ContextBase::testType ContextBase::fileHasChrInChromNames(int fileIdx) {
	conventionType::iterator iter = _fileHasChrInChromNames.find(fileIdx);
	if (iter == _fileHasChrInChromNames.end()) {
		return UNTESTED;
	}
	return iter->second;
}

ContextBase::testType ContextBase::fileHasLeadingZeroInChromNames(int fileIdx) {
	conventionType::iterator iter = _fileHasLeadingZeroInChromNames.find(fileIdx);
	if (iter == _fileHasLeadingZeroInChromNames.end()) {
		return UNTESTED;
	}
	return iter->second;
}

void ContextBase::nameConventionWarning(const Record *record, const QuickString &filename, const QuickString &message)
{
	_nameConventionWarningMsg = "***** WARNING: File ";
	_nameConventionWarningMsg.append(filename);
	_nameConventionWarningMsg.append(message);
	record->print(_nameConventionWarningMsg);
	_nameConventionWarningMsg.append("\n");
	_nameConventionWarningTripped = true;

	cerr << _nameConventionWarningMsg << endl;
}

