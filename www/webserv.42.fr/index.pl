#!/usr/bin/env perl

use strict;
use warnings;
use lib qq($ENV{'LIB_PERL_CGI'});
use CGI;
use CGI::Session;
use CGI::Cookie;
use File::Spec;

my $session_path = $ENV{'CGI_SESSION'};
my $session = new CGI::Session(undef, undef, {Directory=>$session_path});
my $cgi = CGI->new();
my $cookie = $cgi->cookie(CGISESSID => $session->id);

if (!$session->param('username')) {
	$session->param('username', 'guest');
}

if ($ENV{'REQUEST_METHOD'} eq 'POST') {
	if (defined $cgi->param('clearbtn')) {
		$session->delete();
		$session->flush();
		$cookie = CGI::Cookie->new(-name => 'CGISESSID',
			-value => $session->id(), -expires => '-1d');
		print "Location: "
				. "http://$ENV{'HTTP_HOST'}$ENV{'REQUEST_URI'}#welcome\n\n";
	}
	elsif (defined $cgi->param('updatebtn')) {
		my $name = $cgi->param('username');
		$session->param('username', $name);
		print "Location: "
				. "http://$ENV{'HTTP_HOST'}$ENV{'REQUEST_URI'}#welcome\n\n";
	}
	exit();
}

print $cgi->header(-cookie=>$cookie);

# print "session id: ", $session->id(), "<br>";
# print "username: ", $session->param('username');

print <<'END';
<!DOCTYPE html>
<html>
<head>
<title>Webserv on fire!!</title>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="stylesheet" href="https://www.w3schools.com/w3css/4/w3.css">
<link rel="stylesheet" href="https://fonts.googleapis.com/css?family=Lato">
<link rel="stylesheet" href="https://fonts.googleapis.com/css?family=Bitter">
<link rel="stylesheet"
href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css">

<style>
body,h1,h2,h3,h4,h5,h6 {
  font-family: "Bitter", regular;
}

body, html {
  height: 100%;
  color: #777;
  line-height: 1.8;
}

/* Create a Parallax Effect */
.bgimg-1, .bgimg-2, .bgimg-3 {
  background-attachment: fixed;
  background-position: center;
  background-repeat: no-repeat;
  background-size: cover;
}

/* First image (Logo. Full height) */
.bgimg-1 {
  background-image:
    url('https://www.bestjobersblog.com/wp-content/uploads/2019/10/02-Dolomites-Santa-Magdalena-1024x682.jpg');
  min-height: 100%;
  opacity: 0.8;
}

/* Second image (Portfolio) */
.bgimg-2 {
  /* https://images.ctfassets.net/k3n74unfin40/5zZ0QKQMLWu8hEChMpQsx4/3022a954f53ef7e79e1792e7d498f268/GettyImages-1346022120.jpg */
  /* https://www.radiofrance.fr/s3/cruiser-production/2022/05/e3214431-e963-4825-9105-3f7a828ed875/2048x640_gettyimages-1346022120.jpg */
  background-image:
    url("https://ic-cdn.flipboard.com/vox-cdn.com/212e5b215787c1f0d43c8d54180a39efa68167da/_xlarge.jpeg");
  min-height: 400px;
  opacity: 0.8;
}

/* Third image (Contact) */
.bgimg-3 {
  /* https://i.pinimg.com/originals/5c/9d/50/5c9d50600c3e74c375d9d1cd250ebb50.jpg */
  /* https://www.critikat.com/wp-content/uploads/fly-images/203989/349278.jpg-r_1920_1080-f_jpg-q_x-xxyxx-1450x800-c.jpg */
  background-image:
    url("https://i0.wp.com/www.alittlebithuman.com/wp-content/uploads/2021/11/et-and-the-fermi-paradox.png?resize=1170%2C630&ssl=1");
  min-height: 400px;
  opacity: 0.75;
}

/* Turn off parallax scrolling for tablets and phones */
@media only screen and (max-width: 1400px) {
  .bgimg-1, .bgimg-2, .bgimg-3 {
    background-attachment: scroll;
    min-height: 400px;
  }
}

/* Personal additions */

.text-underline {
  --color: #00ffff!important;
  --position: center bottom;
  --width: 50px;
  --height: 1px;
  background: linear-gradient(var(--color), var(--color)) var(--position)
                / var(--width) var(--height) no-repeat;
  padding-bottom: 3px;
}

.w3-wide {
  letter-spacing: 10px;
}

.w3-hover-opacity {
  cursor: pointer;
}

/* Hover effect on portfolio images */
.container {
  position: relative;
  width: 100%;
}

w3-row-padding {
  position: relative;
  table-layout:fixed;
}

.overlay {
  position: absolute;
  top: 0;
  bottom: 0;
  left: 0;
  right: 0;
  height: 100%;
  width: 100%;
  opacity: 0;
  transition: .5s ease;
  visibility: hidden;
}

.container:hover .overlay {
  opacity: 1;
}

.text {
  color: white;
  text-shadow: black 0.005em 0.005em 0.03em;
  font-size: clamp(1em, 2vw, 1.6em);
  position: absolute;
  top: 50%;
  left: 50%;
  -webkit-transform: translate(-50%, -50%);
  -ms-transform: translate(-50%, -50%);
  transform: translate(-50%, -50%);
  text-align: center;
  visibility: visible;
  pointer-events:none;
}

@media (max-width:600px) {
  .text.t0 {
    font-size: 7vw;
  }
}

.w3-hover-opacity:hover {
	opacity: 0.70;
}
</style>

</head>
<body>
<!-- Navbar (sit on top) -->
<div class="w3-top">
  <div class="w3-bar" id="myNavbar">
    <a class="w3-bar-item w3-button w3-hover-black w3-hide-medium w3-hide-large w3-right" href="javascript:void(0);" onclick="toggleFunction()" title="Toggle Navigation Menu">
      <i class="fa fa-bars"></i>
    </a>
    <a href="#home" class="w3-bar-item w3-button"><i class="fa fa-home"></i> HOME</a>
    <a href="#welcome" class="w3-bar-item w3-button w3-hide-small"><i class="fa fa-user"></i> WELCOME</a>
    <a href="#portfolio" class="w3-bar-item w3-button w3-hide-small"><i class="fa fa-th"></i> SCRIPTS</a>
    <a href="#contact" class="w3-bar-item w3-button w3-hide-small"><i class="fa fa-envelope"></i> CONTACT</a>
    <form method="post" target="_self">
      <button class="w3-bar-item w3-button w3-right w3-hide-small" type="submit" name="clearbtn">
        CLEAR SESSION
      </button>
    </form>
  </div>

  <!-- Navbar on small screens -->
  <div id="navDemo" class="w3-bar-block w3-white w3-hide w3-hide-large w3-hide-medium">
    <a href="#welcome" class="w3-bar-item w3-button" onclick="toggleFunction()">WELCOME</a>
    <a href="#portfolio" class="w3-bar-item w3-button" onclick="toggleFunction()">SCRIPTS</a>
    <a href="#contact" class="w3-bar-item w3-button" onclick="toggleFunction()">CONTACT</a>
    <form method="post" target="_self">
      <button class="w3-bar-item w3-button" type="submit" name="clearbtn">
        CLEAR SESSION
      </button>
    </form>
  </div>
</div>

<!-- First Parallax Image with Logo Text -->
<div class="bgimg-1 w3-display-container w3-opacity-min" style="font-family:'Lato', sans-serif;" id="home">
  <div class="w3-display-middle" style="white-space:nowrap;">
    <span class="w3-center w3-padding-large w3-black w3-xlarge w3-wide w3-animate-opacity">WELCOME <span class="w3-hide-small">TO</span> WEBSERV</span>
  </div>
</div>

<!-- Container (Welcome Section) -->
<div class="w3-content w3-container w3-padding-64" id="welcome">
END
print qq(  <h3 class="w3-center w3-text-teal">WELCOME DEAR @{[uc($session->param('username'))]}</h3>

  <p class="w3-center"><em>Do you love surfing?</em></p>
  <div class="w3-row">
    <div class="w3-center w3-padding-large">
      <img src="https://lepetitjournal.com/sites/default/files/2020-02/Kelly%20Slater.jpg" class="w3-round w3-image w3-opacity w3-hover-opacity-off" alt="Photo of Me" width="500" height="333">
    </div>
    <div class="w3-center w3-padding-large">
	  <p>Well, since you're here we guess that you do young @{[$session->param('username')]}!<br><br>But you feel ignored and misunderstood as you're not the one pictured here?<br>Tell us then who you are right away!</p>);
print <<'END';
      <form method="post" target="_self">
        <div class="w3-cell-row" style="margin:8 -16px 8px -16px">
          <div class="w3-third">
            <input class="w3-input" style="pointer-events:none; border-bottom:0px">
          </div>
          <div class="w3-third">
            <input class="w3-input w3-border" type="text" placeholder="My name is ..." name="username" pattern="^(?=.*[A-Za-z0-9]$)[A-Za-z][A-Za-z\d]{0,32}$" title="Must contain 1-32 alphanumeric characters, must start by an alphabetic character" required>
          </div>
        </div>
        <button class="w3-button w3-black w3-center w3-section" type="submit" name="updatebtn">
          <i class="fa fa-refresh"></i>&nbsp; UPDATE
        </button>
      </form>
    </div>
  </div>

  <p class="w3-large w3-center w3-padding-16 w3-text-teal">WE'RE REALLY GOOD AT:</p>
  <p style="letter-spacing:4px; white-space:pre-wrap"><i class="fa fa-code"></i> Imperative & Object-oriented programming</p>
  <div class="w3-light-grey">
    <div class="w3-container w3-padding-small w3-light-blue w3-center" style="width:95%">95%</div>
  </div>
  <p style="letter-spacing:4px; white-space:pre-wrap"><i class="fa fa-code-fork"></i>  Algorithm & Creativity</p>
  <div class="w3-light-grey">
    <div class="w3-container w3-padding-small w3-light-blue w3-center" style="width:90%">90%</div>
  </div>
  <p style="letter-spacing:4px; white-space:pre-wrap"><i class="fa fa-laptop"></i> Network & SysAdmin</p>
  <div class="w3-light-grey">
    <div class="w3-container w3-padding-small w3-light-blue w3-center" style="width:85%">85%</div>
  </div>
</div>

<div class="w3-row w3-center w3-dark-grey w3-padding-16">
  <div class="w3-quarter w3-section">
    <span class="w3-xlarge">3+</span><br>
    Years
  </div>
  <div class="w3-quarter w3-section">
    <span class="w3-xlarge">45+</span><br>
    Projects Done
  </div>
  <div class="w3-quarter w3-section">
    <span class="w3-xlarge">100%</span><br>
    Happy
  </div>
  <div class="w3-quarter w3-section">
    <span class="w3-xlarge">0%</span><br>
    Grumpy
  </div>
</div>

<!-- Second Parallax Image with Portfolio Text -->
<div class="bgimg-2 w3-display-container w3-opacity-min">
  <div class="w3-display-middle">
    <span class="w3-xxlarge w3-text-white w3-wide">LET'S HAVE FUN</span>
  </div>
</div>

<!-- Container (Portfolio Section) -->
<div class="w3-content w3-container w3-padding-64" id="portfolio">
  <h3 class="w3-center w3-text-teal">SCRIPTS</h3>
  <p class="w3-center"><em>Here are some of our home-made recipes!</em></p><br>

  <div class="w3-row-padding w3-center">
    <div class="w3-col m6 w3-border-left w3-border-right w3-border-white w3-hover-border-aqua w3-round-large">
      <h4 class="w3-center w3-text-teal text-underline">Upload File</h4>
      <!-- <form id="uploadForm" action="/_upload/" method="post" enctype="multipart/form-data"  target="hidden-form"> -->
      <form id="uploadForm" action="/upload/" method="post" enctype="multipart/form-data">
        <input type="file" name="file" required onchange="updateUploadAction()">
        <button class="w3-button w3-black w3-section" style="position:relative; top:-1px" type="submit"><i class="fa fa-cloud-upload"></i> UPLOAD</button>
      </form>
      <br>
    </div>

    <iframe style="display:none" name="hidden-form"></iframe>

END
my $dir = File::Spec->catfile($ENV{'DOCUMENT_ROOT'}, 'upload');
my @files;
if (opendir(my $dh, $dir)) {
	@files = grep { !/^\.|upload\.php$/ && !-d $_ } readdir($dh);
	closedir($dh);
}
print <<END;
    <div class="w3-col m6 w3-border-left w3-border-right w3-border-white w3-hover-border-aqua w3-round-large">
      <h4 class="w3-center w3-text-teal text-underline">Download File</h4>
      <form id="download-form" action="/upload/" method="get" onsubmit="handleDownloadFormSubmit(event)">
        <select class="w3-select w3-border" style="width:55%; padding:11.5px" name="file" required onchange="updateDownloadAction(this)">
END
if (@files) {
	print "          <option value='' disabled selected>Choose a file</option>\n";
	foreach my $file (@files) {
		print '          <option value="' . CGI::escape($file) . '">' . $file . "</option>\n";
	}
} else {
	print "          <option value='' disabled selected>No files available for download on the server</option>\n";
}
print <<END;
        </select>
        <button class="w3-button w3-black w3-section" style="position:relative; top:-1px" type="submit" @{[@files ? '' : 'disabled']}><i class="fa fa-cloud-download"></i> DOWNLOAD</button>
      </form>
      <br>
    </div>
END
print <<'END';
  </div>
  <br>
</div>

<!-- Third Parallax Image with Contact Text -->
<div class="bgimg-3 w3-display-container w3-opacity">
  <div class="w3-display-middle">
     <span class="w3-xxlarge w3-text-white w3-wide">CONTACT</span>
  </div>
</div>

<!-- Container (Contact Section) -->
<div class="w3-content w3-container w3-padding-64" id="contact">
  <h3 class="w3-center w3-text-teal">GET IN TOUCH</h3>
  <p class="w3-center"><em>We'd love your feedback!</em></p>

  <div class="w3-row w3-padding-32 w3-section">
    <div class="w3-col m4 w3-container">
      <!-- http://www.knody.com/wp-content/uploads/2014/12/Paris-from-the-sky-1024x676.jpg -->
      <img src="https://static.euronews.com/articles/stories/05/60/71/88/808x1077_cmsv2_47ea77be-3f93-5733-ab72-fdec65af36d7-5607188.jpg" class="w3-image w3-round" style="width:100%; aspect-ratio:9/10; object-fit:cover; opacity:0.99">
    </div>
    <div class="w3-col m8 w3-panel">
      <div class="w3-large w3-margin-bottom">
        <i class="fa fa-map-marker fa-fw w3-hover-text-black w3-xlarge w3-margin-right"></i> Paris, FR<br>
        <i class="fa fa-phone fa-fw w3-hover-text-black w3-xlarge w3-margin-right"></i> Discord: jmazoyer#7419 / (⌐◉_◉)#1446 / Masylk#8775<br>
        <i class="fa fa-envelope fa-fw w3-hover-text-black w3-xlarge w3-margin-right"></i> Email: jmazoyer/flohrel/mtogbe@student.42.fr<br>
      </div>
      <p>Swing by 42 for a cup of <i class="fa fa-coffee"></i>, or leave us a loud & clear note:</p>
      <form id="mailForm" method="get" action="mailto:" onsubmit="handleMailFormSubmit(event)">
        <div class="w3-row-padding" style="margin:0 -16px 8px -16px">
          <div class="w3-half">
            <input class="w3-input w3-border" type="text" placeholder="Subject" required name="subject">
          </div>
          <div class="w3-half">
            <select class="w3-select w3-border" style="padding:11.5px" name="to" required>
              <option value="" disabled selected>Choose your recipient</option>
              <option value="jmazoyer@student.42.fr">jmazoyer@student.42.fr</option>
              <option value="flohrel@student.42.fr">flohrel@student.42.fr</option>
              <option value="mtogbe@student.42.fr">mtogbe@student.42.fr</option>
              <option value="flohrel@student.42.fr,jmazoyer@student.42.fr,mtogbe@student.42.fr">To all</option>
            </select>
          </div>
        </div>
        <textarea class="w3-input w3-border" style="resize:none" placeholder="Message" name="body"></textarea>
        <button class="w3-button w3-black w3-right w3-section" type="submit">
          <i class="fa fa-paper-plane"></i>&nbsp; SEND MESSAGE
        </button>
      </form>
    </div>
  </div>
</div>

<!-- Footer -->
<footer class="w3-center w3-black w3-padding-64 w3-opacity w3-hover-opacity-off">
  <a href="#home" class="w3-button w3-light-grey"><i class="fa fa-arrow-up w3-margin-right"></i>To the top</a>
  <div class="w3-xlarge w3-section">
    <i class="fa fa-github w3-hover-opacity"></i>
    <i class="fa fa-facebook-official w3-hover-opacity"></i>
    <i class="fa fa-instagram w3-hover-opacity"></i>
    <i class="fa fa-pinterest-p w3-hover-opacity"></i>
    <i class="fa fa-twitter w3-hover-opacity"></i>
    <i class="fa fa-linkedin w3-hover-opacity"></i>
  </div>
  <p>Powered by <a href="https://www.w3schools.com/w3css/default.asp" title="W3.CSS" target="_blank" class="w3-hover-text-green">w3.css</a></p>
</footer>

<script>
function updateUploadAction() {
  var filename = document.querySelector('input[type=file]').files[0].name;
  var form = document.getElementById('uploadForm');
  form.action = form.action + filename;
}

function updateDownloadAction(select) {
  var form = document.getElementById('download-form');
  form.action = form.action + select.value;
}

function handleDownloadFormSubmit(event) {
  var form = document.getElementById('download-form');
  event.preventDefault();
  window.location.href = form.action;
}

function handleMailFormSubmit(event) {
  const formData = new FormData(event.target);
  const to = encodeURIComponent(formData.get('to'));
  const subject = encodeURIComponent(formData.get('subject'));
  const body = encodeURIComponent(formData.get('body'));
  const mailtoLink = `mailto:${to}?subject=${subject}&body=${body}`;
  event.preventDefault();
  window.location.href = mailtoLink;
}

// Modal Image Gallery
function onClick(element) {
  document.getElementById("img01").src = element.src;
  document.getElementById("modal01").style.display = "block";
  var captionText = document.getElementById("caption");
  captionText.innerHTML = element.alt;
}

// Change style of navbar on scroll
window.onscroll = function() {myFunction()};
function myFunction() {
  var navbar = document.getElementById("myNavbar");
  if (document.body.scrollTop > 100 || document.documentElement.scrollTop > 100) {
    navbar.className = "w3-bar" + " w3-card" + " w3-animate-top" + " w3-white";
  } else {
    navbar.className = navbar.className.replace(" w3-card w3-animate-top w3-white", "");
  }
}

// Used to toggle the menu on small screens when clicking on the menu button
function toggleFunction() {
  var x = document.getElementById("navDemo");
  if (x.className.indexOf("w3-show") == -1) {
    x.className += " w3-show";
  } else {
    x.className = x.className.replace(" w3-show", "");
  }
}
</script>

</body>
</html>
END
