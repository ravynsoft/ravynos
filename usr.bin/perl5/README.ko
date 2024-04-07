# vim: syntax=pod

이 파일을 내용 그대로 읽고 있다면 우스꽝스러운 문자는 무시해주세요.
이 문서는 POD로 읽을 수 있도록 POD 형식(F<pod/perlpod.pod> 문서를
확인하세요)으로 작성되어 있습니다.

=encoding utf8

=head1 NAME

perlko - 한국어 Perl 안내서

=head1 DESCRIPTION

Perl의 세계에 오신 것을 환영합니다!

Perl은 가끔 B<'Practical Extraction and Report Language'>라고 하기도 합니다만
다른 널리 알려진 것들 중에서 B<'Pathologically Eclectic Rubbish Lister'>라고
하기도 합니다. 사실 이것은 끼워 맞춘 것이며 Perl이 이것들의 첫 글자를
가져와서 이름을 붙인 것은 아닙니다. Perl의 창시자 Larry가 첫 번째 이름을
먼저 생각했고 널리 알려진 것을 나중에 지었기 때문입니다. 그렇기 때문에
B<'Perl'>은 모두 대문자가 아닙니다. 널리 알려진 어떤 것을 가지고 논쟁하는
것은 의미가 없습니다. Larry는 두 개 다 지지합니다.

가끔 p가 소문자로 작성된 B<'perl'>을 볼 것입니다. P가 대문자로 되어 있는
B<'Perl'>은 언어를 참조할 때 쓰이며 B<'perl'>처럼 p가 소문자인 경우는 여러분의
프로그램을 컴파일하고 돌릴 때 사용되는 해석기를 지칭할 때 사용됩니다.


=head1 Perl에 관하여

Perl은 본래 문자열 생성을 위해 만들졌지만 지금은 시스템 관리와 웹 개발,
네트워크 프로그래밍, GUI 개발 등을 포함한 여러 분야에서 널리 사용되는
범용 프로그래밍 언어입니다.

이 언어는 아름다움(아주 작고, 우아하고, 아주 적고)보다
실용적(사용하기 쉽고, 효율적이며, 가능한 최대한)인 것을 지향하고 있습니다.
사용하기 쉽고, 절차적 프로그래밍과 객체 지향 프로그래밍을 모두 지원하고,
강력한 문자열 처리 기능을 내장하고, 세상에서 가장 인상적인 제 3자의 모듈
모음처를 가지고 있다는 것은 Perl의 가장 중요한 특징입니다.

Perl의 언어적 특징은 F<pod/perlintro.pod> 문서에서 소개합니다.

이번 릴리스에서 가장 중요한 변화는 F<pod/perldelta.pod>에서 논의합니다.

또한 다양한 출판사가 출판한 많은 Perl 책은 다양한 주제를 다루고 있습니다.
자세한 정보는 F<pod/perlbook.pod> 문서를 확인하세요.


=head1 설치

여러분이 비교적 현대의 운영체제를 사용하고 있고 현재 버전의 Perl을
지역적으로 설치하고 싶다면 다음 명령을 실행하세요.

    ./Configure -des -Dprefix=$HOME/localperl
    make test
    make install

앞의 명령은 여러분의 플랫폼에 맞게 환경을 설정하고 컴파일을 수행한 후,
회기 테스트를 수행한뒤, 홈 디렉터리 하부의 F<localperl> 디렉터리에 perl을
설치합니다.

여러분이 어떠한 문제든 겪게 되거나 사용자 정의 버전 Perl을 설치할 필요가 있다면
현재 배포판에 들어있는 F<INSTALL> 파일 안의 자세한 설명을 읽어야 합니다.
추가적으로 일반적이지 않은 다양한 플랫폼에서 Perl을 빌드하고 사용하는
방법에 대한 도움말과 귀띔이 적혀있는 많은 수의 F<README> 파일이 있습니다.

일단 Perl을 설치하고 나면 C<perldoc> 도구를 이용해 풍부한 문서를 사용할
수 있습니다. 시작하기 위해서 다음 명령을 실행하세요.

    perldoc perl


=head1 실행에 어려움을 겪는다면

Perl은 뜨개질에서 부터 로켓 과학까지 모든 분야에서 사용할 수 있는 크고
복잡한 시스템입니다. 여러분이 어려움에 부딪혔을때 그 문제는 이미 다른
사람이 해결했을 가능성이 높습니다. 문서를 모두 확인했는데도 버그가
확실하다면 C<perlbug> 도구를 이용해서 저희에게 버그를 보고해주세요.
C<perlbug>에 대한 더 자세한 정보는 C<perldoc perlbug> 또는 C<perlbug>를
명령줄에서 실행해서 확인할 수 있습니다.

Perl을 사용 가능하게 만들었다 하더라도 Perl은 계속해서 진화하기 때문에
여러분이 맞닥뜨린 버그를 수정했거나 여러분이 유용하다고 생각할법한
새로운 기능이 추가된 좀 더 최신 버전이 있을 수 있습니다.

여러분은 항상 최신 버전의 perl을 CPAN (Comprehensive Perl Archive Network)
사이트 L<http://www.cpan.org/src/> 에서 찾을 수 있습니다.

perl 소스에 간단한 패치를 등록하고 싶다면 F<pod/perlhack.pod> 문서의
B<"SUPER QUICK PATCH GUIDE">를 살펴보세요.

그냥 개인적으로 참고하세요.
제가 이것처럼 멋진 물건을 만든다는 것을 여러분이 알기를 바랍니다.
그것은 제 이야기의 B<"저자(Author)">를 기쁘게하기 때문입니다.
이것이 여러분을 귀찮게 한다면 여러분의 B<"저작(Authorship)">에
대한 생각을 정정해야 할 수도 있습니다. 하지만 어쨌거나 여러분은
Perl을 사용하는데는 문제가 없답니다. :-)

- B<"저자">로부터.


=head1 인코딩

Perl은 5.8.0판부터 유니코드/ISO 10646에 대해 광범위하게 지원합니다.
유니코드 지원의 일환으로 한중일을 비롯한 세계 각국에서
유니코드 이전에 쓰고 있었고 지금도 널리 쓰이고 있는 수많은 인코딩을
지원합니다. 유니코드는 전 세계에서 쓰이는 모든 언어를 위한
표기 체계(유럽의 라틴 알파벳, 키릴 알파벳, 그리스 알파벳, 인도와 동남 아시아의
브라미 계열 스크립트, 아랍 문자, 히브리 문자, 한중일의 한자, 한국어의 한글,
일본어의 가나, 북미 인디안의 표기 체계 등)를 수용하는 것을 목표로 하고
있기 때문에 기존에 쓰이던  각 언어 및 국가 그리고 운영 체계에 고유한
문자 집합과 인코딩에 쓸 수 있는 모든 글자는 물론이고  기존 문자 집합에서
지원하고 있지 않던 아주 많은 글자를  포함하고 있습니다.

Perl은 내부적으로 유니코드를 문자 표현을 위해 사용합니다.
보다 구체적으로 말하면 Perl 스크립트 안에서  UTF-8 문자열을 쓸 수 있고,
각종 함수와 연산자(예를 들어, 정규식, index, substr)가 바이트 단위
대신 유니코드 글자 단위로 동작합니다.
더 자세한 것은 F<pod/perlunicode.pod> 문서를 참고하세요.
유니코드가 널리 보급되기 전에 널리 쓰이고 있었고, 여전히 널리 쓰이고 있는
각국/각 언어별 인코딩으로 입출력을 하고 이들 인코딩으로 된 데이터와 문서를
다루는 것을 돕기 위해 L<Encode> 모듈이 쓰이고 있습니다.
무엇보다 L<Encode> 모듈을 사용하면 수많은 인코딩 사이의 변환을 쉽게 할 수 있습니다.


=head2 Encode 모듈

=head3 지원 인코딩

L<Encode> 모듈은 다음과 같은 한국어 인코딩을 지원합니다.

=over 4

=item * C<euc-kr>

US-ASCII와 KS X 1001을 같이 쓰는 멀티바이트 인코딩으로 흔히
완성형이라고 불림. KS X 2901과 RFC 1557 참고.

=item * C<cp949>

MS-Windows 9x/ME에서 쓰이는 확장 완성형. euc-kr에 8,822자의
한글 음절을 더한 것임. alias는 uhc, windows-949, x-windows-949,
ks_c_5601-1987. 맨 마지막 이름은 적절하지 않은 이름이지만, Microsoft
제품에서 CP949의 의미로 쓰이고 있음.

=item * C<johab>

KS X 1001:1998 부록 3에서 규정한 조합형. 문자 레퍼토리는 cp949와 마찬가지로
US-ASCII와  KS X 1001에 8,822자의 한글 음절을 더한 것으로 인코딩 방식은 전혀 다름.

=item * C<iso-2022-kr>

RFC 1557에서 규정한 한국어 인터넷 메일 교환용 인코딩으로 US-ASCII와
KS X 1001을 레퍼토리로 하는 점에서 euc-kr과 같지만 인코딩 방식이 다름.
1997-8년 경까지 쓰였으나 더 이상 메일 교환에 쓰이지 않음.

=item * C<ksc5601-raw>

KS X 1001(KS C 5601)을 GL(즉, MSB를 0으로 한 경우)에 놓았을 때의 인코딩.
US-ASCII와 결합하지 않고 단독으로 쓰이는 일은 X11 등에서 글꼴
인코딩(ksc5601.1987-0. '0'은 GL을 의미함)으로 쓰이는 것을 제외하고는
거의 없음. KS C 5601은 1997년 KS X 1001로 이름을 바꾸었음. 1998년에는 두
글자(유로화 부호와 등록 상표 부호)가 더해졌음.

=back

=head3 변환 예제

예를 들어, euc-kr 인코딩으로 된 파일을 UTF-8로 변환하려면
명령줄에서 다음처럼 실행합니다.

    perl -Mencoding=euc-kr,STDOUT,utf8 -pe1 < file.euc-kr > file.utf8

반대로 변환할 경우 다음처럼 실행합니다.

    perl -Mencoding=utf8,STDOUT,euc-kr -pe1 < file.utf8 > file.euc-kr

이런 변환을 좀더 편리하게 할 수 있도록 도와주는 F<piconv>가 Perl에
기본으로 들어있습니다. 이 유틸리티는 L<Encode> 모듈을 이용한 순수 Perl
유틸리티로 이름에서 알 수 있듯이 Unix의 C<iconv>를 모델로 한 것입니다.
사용법은 다음과 같습니다.

   piconv -f euc-kr -t utf8 < file.euc-kr > file.utf8
   piconv -f utf8 -t euc-kr < file.utf8 > file.euc-kr

=head3 모범 사례

Perl은 기본적으로 내부에서 UTF-8을 사용하며 Encode 모듈을 통해
다양한 인코딩을 지원하지만 항상 다음 규칙을 지킴으로써 인코딩과
관련한 다양하게 발생할 수 있는 문제의 가능성을 줄이는 것을 추천합니다.

=over 4

=item * 소스 코드는 항상 UTF-8 인코딩으로 저장

=item * 소스 코드 상단에 C<use utf8;> 프라그마 사용

=item * 소스 코드, 터미널, 운영체제, 데이터 인코딩을 분리해서 이해

=item * 입출력 파일 핸들에 명시적인 인코딩을 사용

=item * 중복(double) 인코딩을 주의

=back


=head3 유니코드 및 한국어 인코딩 관련 자료

=over 4

=item * L<perluniintro>

=item * L<perlunicode>

=item * L<Encode>

=item * L<Encode::KR>

=item * L<encoding>

=item * L<https://www.unicode.org/>

유니코드 컨소시엄

=item * L<http://std.dkuug.dk/JTC1/SC2/WG2>

기본적으로 Unicode와 같은 ISO 표준인  ISO/IEC 10646 UCS(Universal
Character Set)을 만드는 ISO/IEC JTC1/SC2/WG2의 웹 페이지

=item * L<https://www.cl.cam.ac.uk/~mgk25/unicode.html>

유닉스/리눅스 사용자를 위한 UTF-8 및 유니코드 관련 FAQ

=item * L<http://wiki.kldp.org/Translations/html/UTF8-Unicode-KLDP/UTF8-Unicode-KLDP.html>

유닉스/리눅스 사용자를 위한 UTF-8 및 유니코드 관련 FAQ의 한국어 번역

=back


=head1 Perl 관련 자료

다음은 공식적인 Perl 관련 자료중 일부입니다.

=over 4

=item * L<https://www.perl.org/>

Perl 공식 홈페이지

=item * L<https://www.perl.com/>

O'Reilly의 Perl 웹 페이지

=item * L<https://www.cpan.org/>

CPAN - Comprehensive Perl Archive Network, 통합적 Perl 파일 보관 네트워크

=item * L<https://metacpan.org>

메타 CPAN

=item * L<https://lists.perl.org/>

Perl 메일링 리스트

=item * L<http://blogs.perl.org/>

Perl 메타 블로그

=item * L<https://www.perlmonks.org/>

Perl 수도승들을 위한 수도원

=item * L<https://www.pm.org/groups/asia.html>

아시아 지역 Perl 몽거스 모임

=item * L<http://www.perladvent.org/>

Perl 크리스마스 달력

=back


다음은 Perl을 더 깊게 공부하는데 도움을 줄 수 있는 한국어 관련 사이트입니다.

=over 4

=item * L<https://perl.kr/>

한국 Perl 커뮤니티 공식 포털

=item * L<https://doc.perl.kr/>

Perl 문서 한글화 프로젝트

=item * L<https://cafe.naver.com/perlstudy.cafe>

네이버 Perl 카페

=item * L<http://www.perl.or.kr/>

한국 Perl 사용자 모임

=item * L<https://advent.perl.kr>

Seoul.pm Perl 크리스마스 달력 (2010 ~ 2012)

=item * L<http://gypark.pe.kr/wiki/Perl>

GYPARK(Geunyoung Park)의 Perl 관련 한글 문서 저장소

=back


=head1 라이센스

F<README> 파일의 B<'LICENSING'> 항목을 참고하세요.


=head1 AUTHORS

=over

=item * Jarkko Hietaniemi E<lt>jhi@iki.fiE<gt>

=item * 신정식 E<lt>jshin@mailaps.orgE<gt>

=item * 김도형 E<lt>keedi@cpan.orgE<gt>

=back


=cut
