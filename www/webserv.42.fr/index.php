#!/usr/bin/env php

<?php
  if (!isset($_SESSION)) {
    session_start();
  }
  if ($_POST) {
    if (isset($_POST['clearbtn'])) {
      session_unset();
      session_destroy();
      session_write_close();
      setcookie(session_name(), '', 0, null, null, false, true);
      session_regenerate_id(true);
    }
    elseif (isset($_POST['updatebtn'])) {
      $_SESSION['username'] = $_POST['username'];
      unset($_POST);
    }
    // POST/Redirect/GET to avoid form resubmission
    header("Location: {$_SERVER['REQUEST_URI']}", true, 303);
    exit();
  }

  // output a variable for debugging purpose
  function var_dump_pre($mixed = null) {
    echo '<pre>';
    var_dump($mixed);
    echo '</pre>';
    return null;
  }
?>

<!DOCTYPE html>
<html>
<head>
<title>Webserv on fire!!</title>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="stylesheet" href="https://www.w3schools.com/w3css/4/w3.css">
<link rel="stylesheet" href="https://fonts.googleapis.com/css?family=Lato">
<link rel="stylesheet" href="https://fonts.googleapis.com/css?family=Bitter">
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css">
<style>
body,h1,h2,h3,h4,h5,h6 {font-family: "Bitter", regular;}
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
  background-image: url('https://www.bestjobersblog.com/wp-content/uploads/2019/10/02-Dolomites-Santa-Magdalena-1024x682.jpg');
  min-height: 100%;
  opacity: 0.8;
}

/* Second image (Portfolio) */
.bgimg-2 {
  /* https://images.ctfassets.net/k3n74unfin40/5zZ0QKQMLWu8hEChMpQsx4/3022a954f53ef7e79e1792e7d498f268/GettyImages-1346022120.jpg */
  /* https://www.radiofrance.fr/s3/cruiser-production/2022/05/e3214431-e963-4825-9105-3f7a828ed875/2048x640_gettyimages-1346022120.jpg */
  background-image: url("https://ic-cdn.flipboard.com/vox-cdn.com/212e5b215787c1f0d43c8d54180a39efa68167da/_xlarge.jpeg");
  min-height: 400px;
  opacity: 0.8;
}

/* Third image (Contact) */
.bgimg-3 {
  /* https://i.pinimg.com/originals/5c/9d/50/5c9d50600c3e74c375d9d1cd250ebb50.jpg */
  /* https://www.critikat.com/wp-content/uploads/fly-images/203989/349278.jpg-r_1920_1080-f_jpg-q_x-xxyxx-1450x800-c.jpg */
  background-image: url("https://i0.wp.com/www.alittlebithuman.com/wp-content/uploads/2021/11/et-and-the-fermi-paradox.png?resize=1170%2C630&ssl=1");
  min-height: 400px;
  opacity: 0.75;
}

.w3-wide {letter-spacing: 10px;}
.w3-hover-opacity {cursor: pointer;}

/* Turn off parallax scrolling for tablets and phones */
@media only screen and (max-width: 1500px) {
  .bgimg-1, .bgimg-2, .bgimg-3 {
    background-attachment: scroll;
    min-height: 400px;
  }
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
    <a href="#about" class="w3-bar-item w3-button w3-hide-small"><i class="fa fa-user"></i> WELCOME</a>
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
    <a href="#about" class="w3-bar-item w3-button" onclick="toggleFunction()">WELCOME</a>
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

<!-- Container (About Section) -->
<div class="w3-content w3-container w3-padding-64" id="about">
  <h3 class="w3-center w3-text-teal">
    <?php
      echo "WELCOME DEAR ";
      if (isset($_SESSION['username'])) {
        echo htmlspecialchars($_SESSION['username']);
      } else {
        echo "GUEST";
      }
    ?>
  </h3>
  <p class="w3-center"><em>Do you love surfing?</em></p>
  <div class="w3-row">
    <div class="w3-center w3-padding-large">
      <img src="https://lepetitjournal.com/sites/default/files/2020-02/Kelly%20Slater.jpg" class="w3-round w3-image w3-opacity w3-hover-opacity-off" alt="Photo of Me" width="500" height="333">
    </div>
    <div class="w3-center w3-padding-large">
      <p>Well, since you're here we guess that you do young guest!<br><br>But you feel ignored and misunderstood as you're not the one pictured here?<br>Tell us then who you are right away!</p>
       <form method="post" target="_self">
       <!-- <form action="https://nohello.net/en/" target="_blank"> -->
        <div class="w3-cell-row" style="margin:8 -16px 8px -16px">
          <div class="w3-third">
            <input class="w3-input" style="pointer-events:none; border-bottom:0px">
          </div>
          <div class="w3-third">
            <input class="w3-input w3-border" type="text" placeholder="My name is ..." name="username" pattern="^(?=.*[A-Za-z0-9]$)[A-Za-z][A-Za-z\d]{0,32}$" title="Must contain 1-32 alphanumeric characters, must start by an alphabetic character" required>
          </div>
        </div>
        <button class="w3-button w3-black w3-center w3-section" type="submit" name="updatebtn">
          <i class="fa fa-paper-plane"></i> UPDATE
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

  <!-- Responsive Grid. Four columns on tablets, laptops and desktops. Will stack on mobile devices/small screens (100% width) -->
  <div class="w3-row-padding w3-center">
    <div class="w3-col m3 container">
      <a href="https://projects.intra.42.fr/projects/ft_containers/projects_users/2550474" target="_blank" rel="noopener noreferrer">
      <img src="https://www.capsa-container.com/app/uploads/header-trading.jpg" style="width:100%; aspect-ratio:4/3; object-fit:cover" class="w3-hover-opacity" alt="The mist over the mountains">
        <div class="overlay">
		  <div class="text t0">FT_CONTAINERS</div>
        </div>
      </a>
    </div>

    <div class="w3-col m3 container">
      <a href="https://projects.intra.42.fr/projects/42cursus-philosophers/projects_users/2334104" target="_blank" rel="noopener noreferrer">
      <img src="https://greekreporter.com/wp-content/uploads/2021/08/School_of_Athens_Credit_Wikipedia_Public_domain.jpg" style="width:100%; aspect-ratio:4/3; object-fit:cover" class="w3-hover-opacity" alt="Coffee beans">
        <div class="overlay">
		  <div class="text t0">PHILOSOPHERS</div>
        </div>
      </a>
    </div>

    <div class="w3-col m3 container">
      <a href="https://projects.intra.42.fr/projects/42cursus-minishell/projects_users/2183677" target="_blank" rel="noopener noreferrer">
      <!-- https://static.hitek.fr/img/actualite/ill_m/192129607/matrixcode1024x576.jpg -->
      <!-- https://korben.info/app/uploads/2018/02/matrix-code.jpg -->
      <img src="https://www.topsecret.fr/wp-content/uploads/2016/05/matrix_01-800x500_c.jpg" style="width:100%; aspect-ratio:4/3; object-fit:cover" class="w3-hover-opacity" alt="Bear closeup">
        <div class="overlay">
		  <div class="text t0">MINISHELL</div>
        </div>
        </a>
    </div>

    <div class="w3-col m3 container">
      <a href="https://projects.intra.42.fr/projects/42cursus-push_swap/projects_users/2182507" target="_blank" rel="noopener noreferrer">
      <!-- https://games.lol/wp-content/uploads/2019/04/stack-tiny-bricks-illuminates-1024x614.jpg -->
      <!-- https://lh3.googleusercontent.com/wOgPfnuiPxzXOAIiv7Iuhzu5HWbLRuBlAoGCx2PFHfsUBO07vpUGh9MCYyXrZZYA1LM -->
      <!-- https://tcf.admeen.org/category/500/359/400x400/stacking.jpg -->
      <img src="https://api.web.gamepix.com/assets/img/250/250/icon/stack-tower-game.png" style="width:100%; aspect-ratio:4/3; object-fit:cover" class="w3-hover-opacity" alt="Quiet ocean">
        <div class="overlay">
		  <div class="text t0">PUSH_SWAP</div>
        </div>
        </a>
      </div>
  </div>

<!--
  <div class="w3-row-padding w3-center w3-section">
    <div class="w3-col m3 container">
      <a href="https://google.com/" target="_blank" rel="noopener noreferrer">
      <img src="/w3images/p5.jpg" style="width:100%" class="w3-hover-opacity" alt="The mist">
        <div class="overlay">
		  <div class="text">Inception</div>
        </div>
      </a>
    </div>

    <div class="w3-col m3 container">
      <a href="https://google.com/" target="_blank" rel="noopener noreferrer">
      <img src="/w3images/p6.jpg" style="width:100%" class="w3-hover-opacity" alt="My beloved typewriter">
        <div class="overlay">
		  <div class="text">Inception</div>
        </div>
      </a>
    </div>

    <div class="w3-col m3 container">
      <a href="https://google.com/" target="_blank" rel="noopener noreferrer">
      <img src="/w3images/p7.jpg" style="width:100%" class="w3-hover-opacity" alt="Empty ghost train">
        <div class="overlay">
		  <div class="text">Inception</div>
        </div>
      </a>
    </div>

    <div class="w3-col m3 container">
      <a href="https://google.com/" target="_blank" rel="noopener noreferrer">
      <img src="/w3images/p8.jpg" style="width:100%" class="w3-hover-opacity" alt="Sailing">
        <div class="overlay">
		  <div class="text">Inception</div>
        </div>
      </a>
    </div>
    <button class="w3-button w3-padding-large w3-light-grey" style="margin-top:64px">LOAD MORE</button>
  </div>
-->
<br><br>
</div>

<!-- Modal for full size images on click -->
<!--
<div id="modal01" class="w3-modal w3-black" onclick="this.style.display='none'">
  <span class="w3-button w3-large w3-black w3-display-topright" title="Close Modal Image"><i class="fa fa-remove"></i></span>
  <div class="w3-modal-content w3-animate-zoom w3-center w3-transparent w3-padding-64">
    <img id="img01" class="w3-image">
    <p id="caption" class="w3-opacity w3-large"></p>
  </div>
</div>
-->

<!-- Third Parallax Image with Portfolio Text -->
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
      <!-- Create here a form to send a mail with the text typed (with "mailto:")? Or a simple screen printing the text? -->
      <!-- And with "Name" already filled with name of the user if using cookies and/or sessions? -->
      <!-- <form action="/action_page.php" target="_blank"> -->
      <form method="get" action="mailto:">
        <div class="w3-row-padding" style="margin:0 -16px 8px -16px">
          <div class="w3-half">
            <input class="w3-input w3-border" type="text" placeholder="Subject" required name="subject">
          </div>
          <div class="w3-half">
            <select class="w3-select w3-border" name="to" required>
              <option value="" disabled selected>Choose your recipient</option>
              <option value="jmazoyer@student.42.fr">jmazoyer@student.42.fr</option>
              <option value="flohrel@student.42.fr">flohrel@student.42.fr</option>
              <option value="mtogbe@student.42.fr">mtogbe@student.42.fr</option>
              <option value="flohrel@student.42.fr;jmazoyer@student.42.fr;mtogbe@student.42.fr">To all</option>
            </select>
          </div>
        </div>
        <textarea class="w3-input w3-border" style="resize:none" placeholder="Message" name="body"></textarea>
        <button class="w3-button w3-black w3-right w3-section" type="submit">
          <i class="fa fa-paper-plane"></i> SEND MESSAGE
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