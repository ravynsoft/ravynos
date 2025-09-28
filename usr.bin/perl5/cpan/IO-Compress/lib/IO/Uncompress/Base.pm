
package IO::Uncompress::Base ;

use strict ;
use warnings;
use bytes;

our (@ISA, $VERSION, @EXPORT_OK, %EXPORT_TAGS);
@ISA    = qw(IO::File Exporter);


$VERSION = '2.204';

use constant G_EOF => 0 ;
use constant G_ERR => -1 ;

use IO::Compress::Base::Common 2.204 ;

use IO::File ;
use Symbol;
use Scalar::Util ();
use List::Util ();
use Carp ;

%EXPORT_TAGS = ( );
push @{ $EXPORT_TAGS{all} }, @EXPORT_OK ;

sub smartRead
{
    my $self = $_[0];
    my $out = $_[1];
    my $size = $_[2];
    $$out = "" ;

    my $offset = 0 ;
    my $status = 1;


    if (defined *$self->{InputLength}) {
        return 0
            if *$self->{InputLengthRemaining} <= 0 ;
        $size = List::Util::min($size, *$self->{InputLengthRemaining});
    }

    if ( length *$self->{Prime} ) {
        $$out = substr(*$self->{Prime}, 0, $size) ;
        substr(*$self->{Prime}, 0, $size) =  '' ;
        if (length $$out == $size) {
            *$self->{InputLengthRemaining} -= length $$out
                if defined *$self->{InputLength};

            return length $$out ;
        }
        $offset = length $$out ;
    }

    my $get_size = $size - $offset ;

    if (defined *$self->{FH}) {
        if ($offset) {
            # Not using this
            #
            #  *$self->{FH}->read($$out, $get_size, $offset);
            #
            # because the filehandle may not support the offset parameter
            # An example is Net::FTP
            my $tmp = '';
            $status = *$self->{FH}->read($tmp, $get_size) ;
            substr($$out, $offset) = $tmp
                if defined $status && $status > 0 ;
        }
        else
          { $status = *$self->{FH}->read($$out, $get_size) }
    }
    elsif (defined *$self->{InputEvent}) {
        my $got = 1 ;
        while (length $$out < $size) {
            last
                if ($got = *$self->{InputEvent}->($$out, $get_size)) <= 0;
        }

        if (length $$out > $size ) {
            *$self->{Prime} = substr($$out, $size, length($$out));
            substr($$out, $size, length($$out)) =  '';
        }

       *$self->{EventEof} = 1 if $got <= 0 ;
    }
    else {
       no warnings 'uninitialized';
       my $buf = *$self->{Buffer} ;
       $$buf = '' unless defined $$buf ;
       substr($$out, $offset) = substr($$buf, *$self->{BufferOffset}, $get_size);
       if (*$self->{ConsumeInput})
         { substr($$buf, 0, $get_size) = '' }
       else
         { *$self->{BufferOffset} += length($$out) - $offset }
    }

    *$self->{InputLengthRemaining} -= length($$out) #- $offset
        if defined *$self->{InputLength};

    if (! defined $status) {
        $self->saveStatus($!) ;
        return STATUS_ERROR;
    }

    $self->saveStatus(length $$out < 0 ? STATUS_ERROR : STATUS_OK) ;

    return length $$out;
}

sub pushBack
{
    my $self = shift ;

    return if ! defined $_[0] || length $_[0] == 0 ;

    if (defined *$self->{FH} || defined *$self->{InputEvent} ) {
        *$self->{Prime} = $_[0] . *$self->{Prime} ;
        *$self->{InputLengthRemaining} += length($_[0]);
    }
    else {
        my $len = length $_[0];

        if($len > *$self->{BufferOffset}) {
            *$self->{Prime} = substr($_[0], 0, $len - *$self->{BufferOffset}) . *$self->{Prime} ;
            *$self->{InputLengthRemaining} = *$self->{InputLength};
            *$self->{BufferOffset} = 0
        }
        else {
            *$self->{InputLengthRemaining} += length($_[0]);
            *$self->{BufferOffset} -= length($_[0]) ;
        }
    }
}

sub smartSeek
{
    my $self   = shift ;
    my $offset = shift ;
    my $truncate = shift;
    my $position = shift || SEEK_SET;

    # TODO -- need to take prime into account
    *$self->{Prime} = '';
    if (defined *$self->{FH})
      { *$self->{FH}->seek($offset, $position) }
    else {
        if ($position == SEEK_END) {
            *$self->{BufferOffset} = length(${ *$self->{Buffer} }) + $offset ;
        }
        elsif ($position == SEEK_CUR) {
            *$self->{BufferOffset} += $offset ;
        }
        else {
            *$self->{BufferOffset} = $offset ;
        }

        substr(${ *$self->{Buffer} }, *$self->{BufferOffset}) = ''
            if $truncate;
        return 1;
    }
}

sub smartTell
{
    my $self   = shift ;

    if (defined *$self->{FH})
      { return *$self->{FH}->tell() }
    else
      { return *$self->{BufferOffset} }
}

sub smartWrite
{
    my $self   = shift ;
    my $out_data = shift ;

    if (defined *$self->{FH}) {
        # flush needed for 5.8.0
        defined *$self->{FH}->write($out_data, length $out_data) &&
        defined *$self->{FH}->flush() ;
    }
    else {
       my $buf = *$self->{Buffer} ;
       substr($$buf, *$self->{BufferOffset}, length $out_data) = $out_data ;
       *$self->{BufferOffset} += length($out_data) ;
       return 1;
    }
}

sub smartReadExact
{
    return $_[0]->smartRead($_[1], $_[2]) == $_[2];
}

sub smartEof
{
    my ($self) = $_[0];
    local $.;

    return 0 if length *$self->{Prime} || *$self->{PushMode};

    if (defined *$self->{FH})
    {
        # Could use
        #
        #  *$self->{FH}->eof()
        #
        # here, but this can cause trouble if
        # the filehandle is itself a tied handle, but it uses sysread.
        # Then we get into mixing buffered & non-buffered IO,
        # which will cause trouble

        my $info = $self->getErrInfo();

        my $buffer = '';
        my $status = $self->smartRead(\$buffer, 1);
        $self->pushBack($buffer) if length $buffer;
        $self->setErrInfo($info);

        return $status == 0 ;
    }
    elsif (defined *$self->{InputEvent})
     { *$self->{EventEof} }
    else
     { *$self->{BufferOffset} >= length(${ *$self->{Buffer} }) }
}

sub clearError
{
    my $self   = shift ;

    *$self->{ErrorNo}  =  0 ;
    ${ *$self->{Error} } = '' ;
}

sub getErrInfo
{
    my $self   = shift ;

    return [ *$self->{ErrorNo}, ${ *$self->{Error} } ] ;
}

sub setErrInfo
{
    my $self   = shift ;
    my $ref    = shift;

    *$self->{ErrorNo}  =  $ref->[0] ;
    ${ *$self->{Error} } = $ref->[1] ;
}

sub saveStatus
{
    my $self   = shift ;
    my $errno = shift() + 0 ;

    *$self->{ErrorNo}  = $errno;
    ${ *$self->{Error} } = '' ;

    return *$self->{ErrorNo} ;
}


sub saveErrorString
{
    my $self   = shift ;
    my $retval = shift ;

    ${ *$self->{Error} } = shift ;
    *$self->{ErrorNo} = @_ ? shift() + 0 : STATUS_ERROR ;

    return $retval;
}

sub croakError
{
    my $self   = shift ;
    $self->saveErrorString(0, $_[0]);
    croak $_[0];
}


sub closeError
{
    my $self = shift ;
    my $retval = shift ;

    my $errno = *$self->{ErrorNo};
    my $error = ${ *$self->{Error} };

    $self->close();

    *$self->{ErrorNo} = $errno ;
    ${ *$self->{Error} } = $error ;

    return $retval;
}

sub error
{
    my $self   = shift ;
    return ${ *$self->{Error} } ;
}

sub errorNo
{
    my $self   = shift ;
    return *$self->{ErrorNo};
}

sub HeaderError
{
    my ($self) = shift;
    return $self->saveErrorString(undef, "Header Error: $_[0]", STATUS_ERROR);
}

sub TrailerError
{
    my ($self) = shift;
    return $self->saveErrorString(G_ERR, "Trailer Error: $_[0]", STATUS_ERROR);
}

sub TruncatedHeader
{
    my ($self) = shift;
    return $self->HeaderError("Truncated in $_[0] Section");
}

sub TruncatedTrailer
{
    my ($self) = shift;
    return $self->TrailerError("Truncated in $_[0] Section");
}

sub postCheckParams
{
    return 1;
}

sub checkParams
{
    my $self = shift ;
    my $class = shift ;

    my $got = shift || IO::Compress::Base::Parameters::new();

    my $Valid = {
                    'blocksize'     => [IO::Compress::Base::Common::Parse_unsigned, 16 * 1024],
                    'autoclose'     => [IO::Compress::Base::Common::Parse_boolean,  0],
                    'strict'        => [IO::Compress::Base::Common::Parse_boolean,  0],
                    'append'        => [IO::Compress::Base::Common::Parse_boolean,  0],
                    'prime'         => [IO::Compress::Base::Common::Parse_any,      undef],
                    'multistream'   => [IO::Compress::Base::Common::Parse_boolean,  0],
                    'transparent'   => [IO::Compress::Base::Common::Parse_any,      1],
                    'scan'          => [IO::Compress::Base::Common::Parse_boolean,  0],
                    'inputlength'   => [IO::Compress::Base::Common::Parse_unsigned, undef],
                    'binmodeout'    => [IO::Compress::Base::Common::Parse_boolean,  0],
                   #'decode'        => [IO::Compress::Base::Common::Parse_any,      undef],

                   #'consumeinput'  => [IO::Compress::Base::Common::Parse_boolean,  0],

                    $self->getExtraParams(),

                    #'Todo - Revert to ordinary file on end Z_STREAM_END'=> 0,
                    # ContinueAfterEof
                } ;

    $Valid->{trailingdata} = [IO::Compress::Base::Common::Parse_writable_scalar, undef]
        if  *$self->{OneShot} ;

    $got->parse($Valid, @_ )
        or $self->croakError("${class}: " . $got->getError()) ;

    $self->postCheckParams($got)
        or $self->croakError("${class}: " . $self->error()) ;

    return $got;
}

sub _create
{
    my $obj = shift;
    my $got = shift;
    my $append_mode = shift ;

    my $class = ref $obj;
    $obj->croakError("$class: Missing Input parameter")
        if ! @_ && ! $got ;

    my $inValue = shift ;

    *$obj->{OneShot} = 0 ;

    if (! $got)
    {
        $got = $obj->checkParams($class, undef, @_)
            or return undef ;
    }

    my $inType  = whatIsInput($inValue, 1);

    $obj->ckInputParam($class, $inValue, 1)
        or return undef ;

    *$obj->{InNew} = 1;

    $obj->ckParams($got)
        or $obj->croakError("${class}: " . *$obj->{Error});

    if ($inType eq 'buffer' || $inType eq 'code') {
        *$obj->{Buffer} = $inValue ;
        *$obj->{InputEvent} = $inValue
           if $inType eq 'code' ;
    }
    else {
        if ($inType eq 'handle') {
            *$obj->{FH} = $inValue ;
            *$obj->{Handle} = 1 ;

            # Need to rewind for Scan
            *$obj->{FH}->seek(0, SEEK_SET)
                if $got->getValue('scan');
        }
        else {
            no warnings ;
            my $mode = '<';
            $mode = '+<' if $got->getValue('scan');
            *$obj->{StdIO} = ($inValue eq '-');
            *$obj->{FH} = IO::File->new( "$mode $inValue" )
                or return $obj->saveErrorString(undef, "cannot open file '$inValue': $!", $!) ;
        }

        *$obj->{LineNo} = $. = 0;
        setBinModeInput(*$obj->{FH}) ;

        my $buff = "" ;
        *$obj->{Buffer} = \$buff ;
    }

#    if ($got->getValue('decode')) {
#        my $want_encoding = $got->getValue('decode');
#        *$obj->{Encoding} = IO::Compress::Base::Common::getEncoding($obj, $class, $want_encoding);
#    }
#    else {
#        *$obj->{Encoding} = undef;
#    }

    *$obj->{InputLength}       = $got->parsed('inputlength')
                                    ? $got->getValue('inputlength')
                                    : undef ;
    *$obj->{InputLengthRemaining} = $got->getValue('inputlength');
    *$obj->{BufferOffset}      = 0 ;
    *$obj->{AutoClose}         = $got->getValue('autoclose');
    *$obj->{Strict}            = $got->getValue('strict');
    *$obj->{BlockSize}         = $got->getValue('blocksize');
    *$obj->{Append}            = $got->getValue('append');
    *$obj->{AppendOutput}      = $append_mode || $got->getValue('append');
    *$obj->{ConsumeInput}      = $got->getValue('consumeinput');
    *$obj->{Transparent}       = $got->getValue('transparent');
    *$obj->{MultiStream}       = $got->getValue('multistream');

    # TODO - move these two into RawDeflate
    *$obj->{Scan}              = $got->getValue('scan');
    *$obj->{ParseExtra}        = $got->getValue('parseextra')
                                  || $got->getValue('strict')  ;
    *$obj->{Type}              = '';
    *$obj->{Prime}             = $got->getValue('prime') || '' ;
    *$obj->{Pending}           = '';
    *$obj->{Plain}             = 0;
    *$obj->{PlainBytesRead}    = 0;
    *$obj->{InflatedBytesRead} = 0;
    *$obj->{UnCompSize}        = U64->new;
    *$obj->{CompSize}          = U64->new;
    *$obj->{TotalInflatedBytesRead} = 0;
    *$obj->{NewStream}         = 0 ;
    *$obj->{EventEof}          = 0 ;
    *$obj->{ClassName}         = $class ;
    *$obj->{Params}            = $got ;

    if (*$obj->{ConsumeInput}) {
        *$obj->{InNew} = 0;
        *$obj->{Closed} = 0;
        return $obj
    }

    my $status = $obj->mkUncomp($got);

    return undef
        unless defined $status;

    *$obj->{InNew} = 0;
    *$obj->{Closed} = 0;

    return $obj
        if *$obj->{Pause} ;

    if ($status) {
        # Need to try uncompressing to catch the case
        # where the compressed file uncompresses to an
        # empty string - so eof is set immediately.

        my $out_buffer = '';

        $status = $obj->read(\$out_buffer);

        if ($status < 0) {
            *$obj->{ReadStatus} = [ $status, $obj->error(), $obj->errorNo() ];
        }

        $obj->ungetc($out_buffer)
            if length $out_buffer;
    }
    else {
        return undef
            unless *$obj->{Transparent};

        $obj->clearError();
        *$obj->{Type} = 'plain';
        *$obj->{Plain} = 1;
        $obj->pushBack(*$obj->{HeaderPending})  ;
    }

    push @{ *$obj->{InfoList} }, *$obj->{Info} ;

    $obj->saveStatus(STATUS_OK) ;
    *$obj->{InNew} = 0;
    *$obj->{Closed} = 0;

    return $obj;
}

sub ckInputParam
{
    my $self = shift ;
    my $from = shift ;
    my $inType = whatIsInput($_[0], $_[1]);

    $self->croakError("$from: input parameter not a filename, filehandle, array ref or scalar ref")
        if ! $inType ;

#    if ($inType  eq 'filename' )
#    {
#        return $self->saveErrorString(1, "$from: input filename is undef or null string", STATUS_ERROR)
#            if ! defined $_[0] || $_[0] eq ''  ;
#
#        if ($_[0] ne '-' && ! -e $_[0] )
#        {
#            return $self->saveErrorString(1,
#                            "input file '$_[0]' does not exist", STATUS_ERROR);
#        }
#    }

    return 1;
}


sub _inf
{
    my $obj = shift ;

    my $class = (caller)[0] ;
    my $name = (caller(1))[3] ;

    $obj->croakError("$name: expected at least 1 parameters\n")
        unless @_ >= 1 ;

    my $input = shift ;
    my $haveOut = @_ ;
    my $output = shift ;


    my $x = IO::Compress::Base::Validator->new($class, *$obj->{Error}, $name, $input, $output)
        or return undef ;

    push @_, $output if $haveOut && $x->{Hash};

    *$obj->{OneShot} = 1 ;

    my $got = $obj->checkParams($name, undef, @_)
        or return undef ;

    if ($got->parsed('trailingdata'))
    {
#        my $value = $got->valueRef('TrailingData');
#        warn "TD $value ";
#        #$value = $$value;
##                warn "TD $value $$value ";
#
#        return retErr($obj, "Parameter 'TrailingData' not writable")
#            if readonly $$value ;
#
#        if (ref $$value)
#        {
#            return retErr($obj,"Parameter 'TrailingData' not a scalar reference")
#                if ref $$value ne 'SCALAR' ;
#
#            *$obj->{TrailingData} = $$value ;
#        }
#        else
#        {
#            return retErr($obj,"Parameter 'TrailingData' not a scalar")
#                if ref $value ne 'SCALAR' ;
#
#            *$obj->{TrailingData} = $value ;
#        }

        *$obj->{TrailingData} = $got->getValue('trailingdata');
    }

    *$obj->{MultiStream} = $got->getValue('multistream');
    $got->setValue('multistream', 0);

    $x->{Got} = $got ;

#    if ($x->{Hash})
#    {
#        while (my($k, $v) = each %$input)
#        {
#            $v = \$input->{$k}
#                unless defined $v ;
#
#            $obj->_singleTarget($x, $k, $v, @_)
#                or return undef ;
#        }
#
#        return keys %$input ;
#    }

    if ($x->{GlobMap})
    {
        $x->{oneInput} = 1 ;
        foreach my $pair (@{ $x->{Pairs} })
        {
            my ($from, $to) = @$pair ;
            $obj->_singleTarget($x, $from, $to, @_)
                or return undef ;
        }

        return scalar @{ $x->{Pairs} } ;
    }

    if (! $x->{oneOutput} )
    {
        my $inFile = ($x->{inType} eq 'filenames'
                        || $x->{inType} eq 'filename');

        $x->{inType} = $inFile ? 'filename' : 'buffer';

        foreach my $in ($x->{oneInput} ? $input : @$input)
        {
            my $out ;
            $x->{oneInput} = 1 ;

            $obj->_singleTarget($x, $in, $output, @_)
                or return undef ;
        }

        return 1 ;
    }

    # finally the 1 to 1 and n to 1
    return $obj->_singleTarget($x, $input, $output, @_);

    croak "should not be here" ;
}

sub retErr
{
    my $x = shift ;
    my $string = shift ;

    ${ $x->{Error} } = $string ;

    return undef ;
}

sub _singleTarget
{
    my $self      = shift ;
    my $x         = shift ;
    my $input     = shift;
    my $output    = shift;

    my $buff = '';
    $x->{buff} = \$buff ;

    my $fh ;
    if ($x->{outType} eq 'filename') {
        my $mode = '>' ;
        $mode = '>>'
            if $x->{Got}->getValue('append') ;
        $x->{fh} = IO::File->new( "$mode $output" )
            or return retErr($x, "cannot open file '$output': $!") ;
        binmode $x->{fh} ;

    }

    elsif ($x->{outType} eq 'handle') {
        $x->{fh} = $output;
        binmode $x->{fh} ;
        if ($x->{Got}->getValue('append')) {
                seek($x->{fh}, 0, SEEK_END)
                    or return retErr($x, "Cannot seek to end of output filehandle: $!") ;
            }
    }


    elsif ($x->{outType} eq 'buffer' )
    {
        $$output = ''
            unless $x->{Got}->getValue('append');
        $x->{buff} = $output ;
    }

    if ($x->{oneInput})
    {
        defined $self->_rd2($x, $input, $output)
            or return undef;
    }
    else
    {
        for my $element ( ($x->{inType} eq 'hash') ? keys %$input : @$input)
        {
            defined $self->_rd2($x, $element, $output)
                or return undef ;
        }
    }


    if ( ($x->{outType} eq 'filename' && $output ne '-') ||
         ($x->{outType} eq 'handle' && $x->{Got}->getValue('autoclose'))) {
        $x->{fh}->close()
            or return retErr($x, $!);
        delete $x->{fh};
    }

    return 1 ;
}

sub _rd2
{
    my $self      = shift ;
    my $x         = shift ;
    my $input     = shift;
    my $output    = shift;

    my $z = IO::Compress::Base::Common::createSelfTiedObject($x->{Class}, *$self->{Error});

    $z->_create($x->{Got}, 1, $input, @_)
        or return undef ;

    my $status ;
    my $fh = $x->{fh};

    while (1) {

        while (($status = $z->read($x->{buff})) > 0) {
            if ($fh) {
                local $\;
                print $fh ${ $x->{buff} }
                    or return $z->saveErrorString(undef, "Error writing to output file: $!", $!);
                ${ $x->{buff} } = '' ;
            }
        }

        if (! $x->{oneOutput} ) {
            my $ot = $x->{outType} ;

            if ($ot eq 'array')
              { push @$output, $x->{buff} }
            elsif ($ot eq 'hash')
              { $output->{$input} = $x->{buff} }

            my $buff = '';
            $x->{buff} = \$buff;
        }

        last if $status < 0 || $z->smartEof();

        last
            unless *$self->{MultiStream};

        $status = $z->nextStream();

        last
            unless $status == 1 ;
    }

    return $z->closeError(undef)
        if $status < 0 ;

    ${ *$self->{TrailingData} } = $z->trailingData()
        if defined *$self->{TrailingData} ;

    $z->close()
        or return undef ;

    return 1 ;
}

sub TIEHANDLE
{
    return $_[0] if ref($_[0]);
    die "OOPS\n" ;

}

sub UNTIE
{
    my $self = shift ;
}


sub getHeaderInfo
{
    my $self = shift ;
    wantarray ? @{ *$self->{InfoList} } : *$self->{Info};
}

sub readBlock
{
    my $self = shift ;
    my $buff = shift ;
    my $size = shift ;

    if (defined *$self->{CompressedInputLength}) {
        if (*$self->{CompressedInputLengthRemaining} == 0) {
            delete *$self->{CompressedInputLength};
            *$self->{CompressedInputLengthDone} = 1;
            return STATUS_OK ;
        }
        $size = List::Util::min($size, *$self->{CompressedInputLengthRemaining} );
        *$self->{CompressedInputLengthRemaining} -= $size ;
    }

    my $status = $self->smartRead($buff, $size) ;
    return $self->saveErrorString(STATUS_ERROR, "Error Reading Data: $!", $!)
        if $status == STATUS_ERROR  ;

    if ($status == 0 ) {
        *$self->{Closed} = 1 ;
        *$self->{EndStream} = 1 ;
        return $self->saveErrorString(STATUS_ERROR, "unexpected end of file", STATUS_ERROR);
    }

    return STATUS_OK;
}

sub postBlockChk
{
    return STATUS_OK;
}

sub _raw_read
{
    # return codes
    # >0 - ok, number of bytes read
    # =0 - ok, eof
    # <0 - not ok

    my $self = shift ;

    return G_EOF if *$self->{Closed} ;
    return G_EOF if *$self->{EndStream} ;

    my $buffer = shift ;
    my $scan_mode = shift ;

    if (*$self->{Plain}) {
        my $tmp_buff ;
        my $len = $self->smartRead(\$tmp_buff, *$self->{BlockSize}) ;

        return $self->saveErrorString(G_ERR, "Error reading data: $!", $!)
                if $len == STATUS_ERROR ;

        if ($len == 0 ) {
            *$self->{EndStream} = 1 ;
        }
        else {
            *$self->{PlainBytesRead} += $len ;
            $$buffer .= $tmp_buff;
        }

        return $len ;
    }

    if (*$self->{NewStream}) {

        $self->gotoNextStream() > 0
            or return G_ERR;

        # For the headers that actually uncompressed data, put the
        # uncompressed data into the output buffer.
        $$buffer .=  *$self->{Pending} ;
        my $len = length  *$self->{Pending} ;
        *$self->{Pending} = '';
        return $len;
    }

    my $temp_buf = '';
    my $outSize = 0;
    my $status = $self->readBlock(\$temp_buf, *$self->{BlockSize}, $outSize) ;

    return G_ERR
        if $status == STATUS_ERROR  ;

    my $buf_len = 0;
    if ($status == STATUS_OK) {
        my $beforeC_len = length $temp_buf;
        my $before_len = defined $$buffer ? length $$buffer : 0 ;
        $status = *$self->{Uncomp}->uncompr(\$temp_buf, $buffer,
                                    defined *$self->{CompressedInputLengthDone} ||
                                                $self->smartEof(), $outSize);

        # Remember the input buffer if it wasn't consumed completely
        $self->pushBack($temp_buf) if *$self->{Uncomp}{ConsumesInput};

        return $self->saveErrorString(G_ERR, *$self->{Uncomp}{Error}, *$self->{Uncomp}{ErrorNo})
            if $self->saveStatus($status) == STATUS_ERROR;

        $self->postBlockChk($buffer, $before_len) == STATUS_OK
            or return G_ERR;

        $buf_len = defined $$buffer ? length($$buffer) - $before_len : 0;

        *$self->{CompSize}->add($beforeC_len - length $temp_buf) ;

        *$self->{InflatedBytesRead} += $buf_len ;
        *$self->{TotalInflatedBytesRead} += $buf_len ;
        *$self->{UnCompSize}->add($buf_len) ;

        $self->filterUncompressed($buffer, $before_len);

#        if (*$self->{Encoding}) {
#            use Encode ;
#            *$self->{PendingDecode} .= substr($$buffer, $before_len) ;
#            my $got = *$self->{Encoding}->decode(*$self->{PendingDecode}, Encode::FB_QUIET) ;
#            substr($$buffer, $before_len) = $got;
#        }
    }

    if ($status == STATUS_ENDSTREAM) {

        *$self->{EndStream} = 1 ;

        my $trailer;
        my $trailer_size = *$self->{Info}{TrailerLength} ;
        my $got = 0;
        if (*$self->{Info}{TrailerLength})
        {
            $got = $self->smartRead(\$trailer, $trailer_size) ;
        }

        if ($got == $trailer_size) {
            $self->chkTrailer($trailer) == STATUS_OK
                or return G_ERR;
        }
        else {
            return $self->TrailerError("trailer truncated. Expected " .
                                      "$trailer_size bytes, got $got")
                if *$self->{Strict};
            $self->pushBack($trailer)  ;
        }

        # TODO - if want file pointer, do it here

        if (! $self->smartEof()) {
            *$self->{NewStream} = 1 ;

            if (*$self->{MultiStream}) {
                *$self->{EndStream} = 0 ;
                return $buf_len ;
            }
        }

    }


    # return the number of uncompressed bytes read
    return $buf_len ;
}

sub reset
{
    my $self = shift ;

    return *$self->{Uncomp}->reset();
}

sub filterUncompressed
{
}

#sub isEndStream
#{
#    my $self = shift ;
#    return *$self->{NewStream} ||
#           *$self->{EndStream} ;
#}

sub nextStream
{
    my $self = shift ;

    # An uncompressed file cannot have a next stream, so
    # return immediately.
    return 0
        if *$self->{Plain} ;

    my $status = $self->gotoNextStream();
    $status == 1
        or return $status ;

    *$self->{Pending} = ''
        if $self !~ /IO::Uncompress::RawInflate/ && ! *$self->{MultiStream};

    *$self->{TotalInflatedBytesRead} = 0 ;
    *$self->{LineNo} = $. = 0;

    return 1;
}

sub gotoNextStream
{
    my $self = shift ;

    if (! *$self->{NewStream}) {
        my $status = 1;
        my $buffer ;

        # TODO - make this more efficient if know the offset for the end of
        # the stream and seekable
        $status = $self->read($buffer)
            while $status > 0 ;

        return $status
            if $status < 0;
    }

    *$self->{NewStream} = 0 ;
    *$self->{EndStream} = 0 ;
    *$self->{CompressedInputLengthDone} = undef ;
    *$self->{CompressedInputLength} = undef ;
    $self->reset();
    *$self->{UnCompSize}->reset();
    *$self->{CompSize}->reset();

    my $magic = $self->ckMagic();

    if ( ! defined $magic) {
        if (! *$self->{Transparent} || $self->eof())
        {
            *$self->{EndStream} = 1 ;
            return 0;
        }

        # Not EOF, so Transparent mode kicks in now for trailing data
        # Reset member name in case anyone calls getHeaderInfo()->{Name}
        *$self->{Info} = { Name => undef, Type  => 'plain' };

        $self->clearError();
        *$self->{Type} = 'plain';
        *$self->{Plain} = 1;
        $self->pushBack(*$self->{HeaderPending})  ;
    }
    else
    {
        *$self->{Info} = $self->readHeader($magic);

        if ( ! defined *$self->{Info} ) {
            *$self->{EndStream} = 1 ;
            return -1;
        }
    }

    push @{ *$self->{InfoList} }, *$self->{Info} ;

    return 1;
}

sub streamCount
{
    my $self = shift ;
    return 1 if ! defined *$self->{InfoList};
    return scalar @{ *$self->{InfoList} }  ;
}

sub read
{
    # return codes
    # >0 - ok, number of bytes read
    # =0 - ok, eof
    # <0 - not ok

    my $self = shift ;

    if (defined *$self->{ReadStatus} ) {
        my $status = *$self->{ReadStatus}[0];
        $self->saveErrorString( @{ *$self->{ReadStatus} } );
        delete  *$self->{ReadStatus} ;
        return $status ;
    }

    return G_EOF if *$self->{Closed} ;

    my $buffer ;

    if (ref $_[0] ) {
        $self->croakError(*$self->{ClassName} . "::read: buffer parameter is read-only")
            if Scalar::Util::readonly(${ $_[0] });

        $self->croakError(*$self->{ClassName} . "::read: not a scalar reference $_[0]" )
            unless ref $_[0] eq 'SCALAR' ;
        $buffer = $_[0] ;
    }
    else {
        $self->croakError(*$self->{ClassName} . "::read: buffer parameter is read-only")
            if Scalar::Util::readonly($_[0]);

        $buffer = \$_[0] ;
    }

    my $length = $_[1] ;
    my $offset = $_[2] || 0;

    if (! *$self->{AppendOutput}) {
        if (! $offset) {

            $$buffer = '' ;
        }
        else {
            if ($offset > length($$buffer)) {
                $$buffer .= "\x00" x ($offset - length($$buffer));
            }
            else {
                substr($$buffer, $offset) = '';
            }
        }
    }
    elsif (! defined $$buffer) {
        $$buffer = '' ;
    }

    return G_EOF if !length *$self->{Pending} && *$self->{EndStream} ;

    # the core read will return 0 if asked for 0 bytes
    return 0 if defined $length && $length == 0 ;

    $length = $length || 0;

    $self->croakError(*$self->{ClassName} . "::read: length parameter is negative")
        if $length < 0 ;

    # Short-circuit if this is a simple read, with no length
    # or offset specified.
    unless ( $length || $offset) {
        if (length *$self->{Pending}) {
            $$buffer .= *$self->{Pending} ;
            my $len = length *$self->{Pending};
            *$self->{Pending} = '' ;
            return $len ;
        }
        else {
            my $len = 0;
            $len = $self->_raw_read($buffer)
                while ! *$self->{EndStream} && $len == 0 ;
            return $len ;
        }
    }

    # Need to jump through more hoops - either length or offset
    # or both are specified.
    my $out_buffer = *$self->{Pending} ;
    *$self->{Pending} = '';


    while (! *$self->{EndStream} && length($out_buffer) < $length)
    {
        my $buf_len = $self->_raw_read(\$out_buffer);
        return $buf_len
            if $buf_len < 0 ;
    }

    $length = length $out_buffer
        if length($out_buffer) < $length ;

    return 0
        if $length == 0 ;

    $$buffer = ''
        if ! defined $$buffer;

    $offset = length $$buffer
        if *$self->{AppendOutput} ;

    *$self->{Pending} = $out_buffer;
    $out_buffer = \*$self->{Pending} ;

    substr($$buffer, $offset) = substr($$out_buffer, 0, $length) ;
    substr($$out_buffer, 0, $length) =  '' ;

    return $length ;
}

sub _getline
{
    my $self = shift ;
    my $status = 0 ;

    # Slurp Mode
    if ( ! defined $/ ) {
        my $data ;
        1 while ($status = $self->read($data)) > 0 ;
        return ($status, \$data);
    }

    # Record Mode
    if ( ref $/ eq 'SCALAR' && ${$/} =~ /^\d+$/ && ${$/} > 0) {
        my $reclen = ${$/} ;
        my $data ;
        $status = $self->read($data, $reclen) ;
        return ($status, \$data);
    }

    # Paragraph Mode
    if ( ! length $/ ) {
        my $paragraph ;
        while (($status = $self->read($paragraph)) > 0 ) {
            if ($paragraph =~ s/^(.*?\n\n+)//s) {
                *$self->{Pending}  = $paragraph ;
                my $par = $1 ;
                return (1, \$par);
            }
        }
        return ($status, \$paragraph);
    }

    # $/ isn't empty, or a reference, so it's Line Mode.
    {
        my $line ;
        my $p = \*$self->{Pending}  ;
        while (($status = $self->read($line)) > 0 ) {
            my $offset = index($line, $/);
            if ($offset >= 0) {
                my $l = substr($line, 0, $offset + length $/ );
                substr($line, 0, $offset + length $/) = '';
                $$p = $line;
                return (1, \$l);
            }
        }

        return ($status, \$line);
    }
}

sub getline
{
    my $self = shift;

    if (defined *$self->{ReadStatus} ) {
        $self->saveErrorString( @{ *$self->{ReadStatus} } );
        delete  *$self->{ReadStatus} ;
        return undef;
    }

    return undef
        if *$self->{Closed} || (!length *$self->{Pending} && *$self->{EndStream}) ;

    my $current_append = *$self->{AppendOutput} ;
    *$self->{AppendOutput} = 1;

    my ($status, $lineref) = $self->_getline();
    *$self->{AppendOutput} = $current_append;

    return undef
        if $status < 0 || length $$lineref == 0 ;

    $. = ++ *$self->{LineNo} ;

    return $$lineref ;
}

sub getlines
{
    my $self = shift;
    $self->croakError(*$self->{ClassName} .
            "::getlines: called in scalar context\n") unless wantarray;
    my($line, @lines);
    push(@lines, $line)
        while defined($line = $self->getline);
    return @lines;
}

sub READLINE
{
    goto &getlines if wantarray;
    goto &getline;
}

sub getc
{
    my $self = shift;
    my $buf;
    return $buf if $self->read($buf, 1);
    return undef;
}

sub ungetc
{
    my $self = shift;
    *$self->{Pending} = ""  unless defined *$self->{Pending} ;
    *$self->{Pending} = $_[0] . *$self->{Pending} ;
}


sub trailingData
{
    my $self = shift ;

    if (defined *$self->{FH} || defined *$self->{InputEvent} ) {
        return *$self->{Prime} ;
    }
    else {
        my $buf = *$self->{Buffer} ;
        my $offset = *$self->{BufferOffset} ;
        return substr($$buf, $offset) ;
    }
}


sub eof
{
    my $self = shift ;

    return (*$self->{Closed} ||
              (!length *$self->{Pending}
                && ( $self->smartEof() || *$self->{EndStream}))) ;
}

sub tell
{
    my $self = shift ;

    my $in ;
    if (*$self->{Plain}) {
        $in = *$self->{PlainBytesRead} ;
    }
    else {
        $in = *$self->{TotalInflatedBytesRead} ;
    }

    my $pending = length *$self->{Pending} ;

    return 0 if $pending > $in ;
    return $in - $pending ;
}

sub close
{
    # todo - what to do if close is called before the end of the gzip file
    #        do we remember any trailing data?
    my $self = shift ;

    return 1 if *$self->{Closed} ;

    untie *$self
        if $] >= 5.008 ;

    my $status = 1 ;

    if (defined *$self->{FH}) {
        if ((! *$self->{Handle} || *$self->{AutoClose}) && ! *$self->{StdIO}) {
            local $.;
            $! = 0 ;
            $status = *$self->{FH}->close();
            return $self->saveErrorString(0, $!, $!)
                if !*$self->{InNew} && $self->saveStatus($!) != 0 ;
        }
        delete *$self->{FH} ;
        $! = 0 ;
    }
    *$self->{Closed} = 1 ;

    return 1;
}

sub DESTROY
{
    my $self = shift ;
    local ($., $@, $!, $^E, $?);

    $self->close() ;
}

sub seek
{
    my $self     = shift ;
    my $position = shift;
    my $whence   = shift ;

    my $here = $self->tell() ;
    my $target = 0 ;


    if ($whence == SEEK_SET) {
        $target = $position ;
    }
    elsif ($whence == SEEK_CUR) {
        $target = $here + $position ;
    }
    elsif ($whence == SEEK_END) {
        $target = $position ;
        $self->croakError(*$self->{ClassName} . "::seek: SEEK_END not allowed") ;
    }
    else {
        $self->croakError(*$self->{ClassName} ."::seek: unknown value, $whence, for whence parameter");
    }

    # short circuit if seeking to current offset
    if ($target == $here) {
        # On ordinary filehandles, seeking to the current
        # position also clears the EOF condition, so we
        # emulate this behavior locally while simultaneously
        # cascading it to the underlying filehandle
        if (*$self->{Plain}) {
            *$self->{EndStream} = 0;
            seek(*$self->{FH},0,1) if *$self->{FH};
        }
        return 1;
    }

    # Outlaw any attempt to seek backwards
    $self->croakError( *$self->{ClassName} ."::seek: cannot seek backwards")
        if $target < $here ;

    # Walk the file to the new offset
    my $offset = $target - $here ;

    my $got;
    while (($got = $self->read(my $buffer, List::Util::min($offset, *$self->{BlockSize})) ) > 0)
    {
        $offset -= $got;
        last if $offset == 0 ;
    }

    $here = $self->tell() ;
    return $offset == 0 ? 1 : 0 ;
}

sub fileno
{
    my $self = shift ;
    return defined *$self->{FH}
           ? fileno *$self->{FH}
           : undef ;
}

sub binmode
{
    1;
#    my $self     = shift ;
#    return defined *$self->{FH}
#            ? binmode *$self->{FH}
#            : 1 ;
}

sub opened
{
    my $self     = shift ;
    return ! *$self->{Closed} ;
}

sub autoflush
{
    my $self     = shift ;
    return defined *$self->{FH}
            ? *$self->{FH}->autoflush(@_)
            : undef ;
}

sub input_line_number
{
    my $self = shift ;
    my $last = *$self->{LineNo};
    $. = *$self->{LineNo} = $_[1] if @_ ;
    return $last;
}

sub _notAvailable
{
    my $name = shift ;
    return sub { croak "$name Not Available: File opened only for intput" ; } ;
}

{
    no warnings 'once';

    *BINMODE  = \&binmode;
    *SEEK     = \&seek;
    *READ     = \&read;
    *sysread  = \&read;
    *TELL     = \&tell;
    *EOF      = \&eof;

    *FILENO   = \&fileno;
    *CLOSE    = \&close;

    *print    = _notAvailable('print');
    *PRINT    = _notAvailable('print');
    *printf   = _notAvailable('printf');
    *PRINTF   = _notAvailable('printf');
    *write    = _notAvailable('write');
    *WRITE    = _notAvailable('write');

    #*sysread  = \&read;
    #*syswrite = \&_notAvailable;
}



package IO::Uncompress::Base ;


1 ;
__END__

=head1 NAME

IO::Uncompress::Base - Base Class for IO::Uncompress modules

=head1 SYNOPSIS

    use IO::Uncompress::Base ;

=head1 DESCRIPTION

This module is not intended for direct use in application code. Its sole
purpose is to be sub-classed by IO::Uncompress modules.

=head1 SUPPORT

General feedback/questions/bug reports should be sent to
L<https://github.com/pmqs/IO-Compress/issues> (preferred) or
L<https://rt.cpan.org/Public/Dist/Display.html?Name=IO-Compress>.

=head1 SEE ALSO

L<Compress::Zlib>, L<IO::Compress::Gzip>, L<IO::Uncompress::Gunzip>, L<IO::Compress::Deflate>, L<IO::Uncompress::Inflate>, L<IO::Compress::RawDeflate>, L<IO::Uncompress::RawInflate>, L<IO::Compress::Bzip2>, L<IO::Uncompress::Bunzip2>, L<IO::Compress::Lzma>, L<IO::Uncompress::UnLzma>, L<IO::Compress::Xz>, L<IO::Uncompress::UnXz>, L<IO::Compress::Lzip>, L<IO::Uncompress::UnLzip>, L<IO::Compress::Lzop>, L<IO::Uncompress::UnLzop>, L<IO::Compress::Lzf>, L<IO::Uncompress::UnLzf>, L<IO::Compress::Zstd>, L<IO::Uncompress::UnZstd>, L<IO::Uncompress::AnyInflate>, L<IO::Uncompress::AnyUncompress>

L<IO::Compress::FAQ|IO::Compress::FAQ>

L<File::GlobMapper|File::GlobMapper>, L<Archive::Zip|Archive::Zip>,
L<Archive::Tar|Archive::Tar>,
L<IO::Zlib|IO::Zlib>

=head1 AUTHOR

This module was written by Paul Marquess, C<pmqs@cpan.org>.

=head1 MODIFICATION HISTORY

See the Changes file.

=head1 COPYRIGHT AND LICENSE

Copyright (c) 2005-2023 Paul Marquess. All rights reserved.

This program is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.
