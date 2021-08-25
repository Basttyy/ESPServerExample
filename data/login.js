const loginModal = document.querySelector('.loginModal');

const closeModal = () => {
    if (loginModal.style.display === "block") {
        loginModal.style.display = "none"
    } else {
        loginModal.style.display = "block"
    }
}

const showModal = () => {
    if (loginModal.style.display === "none") {
      loginModal.style.display = "block";
    } else {
      loginModal.style.display = "none";
    }
};

$(document).ready(function() {
    headar = document.getElementById('alertheader');
    bodi = document.getElementById('alertbody');
    const urlSearchParams = new URLSearchParams(window.location.search);
    const message = urlSearchParams.get('msg');
    document.getElementById('activenav').value() = urlSearchParams.get('activenav');

    if (urlSearchParams.has('msg') && message != '') {
        headar.innerHtml = "Alert";
        bodi.innerHtml = message;
        showModal();
    }
});