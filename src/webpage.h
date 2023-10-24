
const char* html_start_doc = R"(
 <!DOCTYPE html>
 <html>
)";

const char* html_end_doc = R"(
 </html>
)";

const char* html_header = R"(

<head>
  <title>Captive Portal</title>
  <style>
    body {
      font-family: sans-serif;
      margin: 0;
      padding: 0;
    }
    .container {
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      height: 100vh;
    }
    h1 {
      font-size: 3rem;
      margin-bottom: 1rem;
    }
    p {
      font-size: 1.5rem;
      margin-bottom: 2rem;
    }
    input[type="text"], input[type="password"] {
      padding: 0.5rem;
      margin-bottom: 1rem;
      border-radius: 0.25rem;
      border: none;
    }
    button {
      padding: 0.5rem 1rem;
      border-radius: 0.25rem;
      border: none;
      background-color: #0078d7;
      color: white;
      font-size: 1.0rem;
    }
    img {
      border-radius: 0.25rem;
      border: none;
    }
  </style>
</head>
)";

const char * html_body = R"(
<body>
 <div class="container">
  <h1>Wifi configuratie</h1>
    <p>Log in:</p>
	<form method="post" action="/login">
      <input type="text" name="username" placeholder="Username">
      <input type="password" name="password" placeholder="Password">
      <button type="submit">Connect</button>
    </form>
  </div>
</body>
)";

