const smsModal = document.querySelector(".smsModal")
const smsDiv = document.querySelector(".smsDiv")
const errorDiv = document.querySelector(".errorDiv")
const otaDiv = document.querySelector(".otaDiv")
const navItem1 = document.querySelector(".navItem1")
const navItem2 = document.querySelector(".navItem2")
const navItem3 = document.querySelector(".navItem3")
const pageHeader = document.querySelector("#pageHeader")
var smsTable
var errorTable
let base_url = window.location.origin
let data_url = base_url + "/api/table-data"
let error_url = base_url + '/api/error'
let sms_url = base_url + '/api/sms'

if (!!window.EventSource) {
    var source = new EventSource('/events');
    
    source.addEventListener('open', function(e) {
     console.log("Events Connected");
    }, false);

    source.addEventListener('error', function(e) {
        if (e.target.readyState != EventSource.OPEN) {
        console.log("Events Disconnected");
        }
    }, false);

    source.addEventListener('ping', function(e) {
        console.log("Ping Event", e.data);
    });

    source.addEventListener('exception', function(e) {
        let error = []
        data = json.parse(e.data)
        console.log("exception event: ", data.message);
        error.push(data.id)
        error.push(data.message)
        error.push(data.trace)
        error.push(data.time)

        errorDataSet = JSON.parse(localStorage.getItem('errorData'))
        localStorage.removeItem('errorData')
        errorDataSet.push(error)
        localStorage.setItem('errorData', JSON.stringify(errorDataSet))
        errorTable.row.add(error).draw()
    })
    
    source.addEventListener('sms', function(e) {
     console.log("sms event: ", e.data);
        let sms = []
        //var message = base64Decode(e.data)
        // message = message.replace('aaaa', '')
        // message = message.replace('\n', '')
        // message = message.replace('\r', '')
        // message = message.replace('OK', '')
        // items = message.split(',')
        data = JSON.parse(e.data)

        sms.push(data.id)
        sms.push(data.address)
        sms.push(data.name)
        sms.push(data.message)
        sms.push(data.time)

        smsDataSet = JSON.parse(localStorage.getItem('smsData'))
        localStorage.removeItem('smsData')
        smsDataSet.push(sms)
        localStorage.setItem('smsData', JSON.stringify(smsDataSet))
        smsTable.row.add(sms).draw()

        // if (items.length > 4) {
        //     items_array = items[3].split("+04")

        //     items[3] = items_array[0]
        //     items[4] = items_array[1]

            
        //     smsDataSet = JSON.parse(localStorage.getItem('smsData'))
        //     localStorage.removeItem('smsData')
        //     sms.push(e.data.id)
        //     sms.push(items[1].replace('\"', ''))
        //     sms.push(items[2].replace('\"', ''))
        //     sms.push('20'+ items[3].replace('-', ''))
        //     sms.push(items[4].replace('\"', ''))
        //     smsDataSet.push(sms)
        //     localStorage.setItem('smsData', JSON.stringify(smsDataSet))
        //     smsTable.row.add(sms).draw()
        // }
    }, false);
}

const displaySMSDetails = () => {
    if (smsModal.style.display === "none") {
        smsModal.style.display = "block"
    } else {
        smsModal.style.display = "none"
    }
}

const closeModal = () => {
    if (smsModal.style.display === "block") {
        smsModal.style.display = "none"
    } else {
        smsModal.style.display = "block"
    }
}

const displaySMSList = () => {
    if (smsDiv.style.display === "none") {
        pageHeader.innerHTML = "SMS List"
        smsDiv.style.display = "block"
        navItem2.classList.remove("clicked")
        navItem3.classList.remove("clicked")
        navItem1.classList.add("clicked")
        errorDiv.style.display = "none"
        otaDiv.style.display = "none"
    } else {
        navItem1.classList.add("clicked")
    }
}

const displayErrorList = () => {
    if (errorDiv.style.display === "none") {
        pageHeader.innerHTML = "Error List"
        errorDiv.style.display = "block"
        navItem1.classList.remove("clicked")
        navItem3.classList.remove("clicked")
        navItem2.classList.add("clicked")
        smsDiv.style.display = "none"
        otaDiv.style.display = "none"
    } else {
        //errorDiv.style.display = "none"
    }
}

const displayOtaForm = () => {
    if (otaDiv.style.display === "none") {
        pageHeader.innerHTML = "Firmware Update"
        otaDiv.style.display = "block"
        navItem1.classList.remove("clicked")
        navItem2.classList.remove("clicked")
        navItem3.classList.add("clicked")
        errorDiv.style.display = "none"
        smsDiv.style.display = "none"
    } else {
        //navItem1.classList.add("clicked")
    }
}


// const smsTitle = document.getElementById("smsTitle");
// const smsText = document.getElementById("smsText");
// const smsDetailsContainer = document.getElementById("smsDetails")
// const paginationNumbers = document.getElementById("paginationNumbers")
const dropdownIcon = document.getElementById("dropdownIcon");
const hideNavbar = document.getElementById("hideNavbar");

const urlOrigin = window.location.origin
const urlPath = window.location.pathname

const showDropdown = () => {
    hideNavbar.classList.toggle("showNavbar");
};

/*
*  Datatable starts here
*/

let getCookie = (cname) => {
    let name = cname + "=";
    let ca = document.cookie.split(';');
    for(let i = 0; i < ca.length; i++) {
      let c = ca[i];
      while (c.charAt(0) == ' ') {
        c = c.substring(1);
      }
      if (c.indexOf(name) == 0) {
        return c.substring(name.length, c.length);
      }
    }
    return "";
}

function base64Encode( str ) {
    return window.btoa(unescape(encodeURIComponent( str )));
}

function base64Decode( str ) {
    return decodeURIComponent(escape(window.atob( str )));
}

$(document).ready(function() {
    const urlSearchParams = new URLSearchParams(window.location.search);
    const activeNav = urlSearchParams.get('activenav');

    switch (activeNav) {
        case 'sms':
            smsDiv.style.display = "block"
            errorDiv.style.display = "none"
            otaDiv.style.display = "none"
            displaySMSList();
            break;
        case 'error':
            errorDiv.style.display = "block"
            smsDiv.style.display = "none"
            otaDiv.style.display = "none"
            displayErrorList();
            break;
        case 'ota':
            otaDiv.style.display = "block"
            errorDiv.style.display = "none"
            smsDiv.style.display = "none"
            displayOtaForm();
            break;
        default:
            smsDiv.style.display = "block"
            errorDiv.style.display = "none"
            otaDiv.style.display = "none"
            displaySMSList();
    }

    fetch (data_url, {
        method: 'GET',
        mode: 'cors',
        cache: 'no-cache',
        credentials: 'same-origin',
        headers: {
            'Content-Type': 'application/json',
            'Cookie': "ESPSESSIONID=" + getCookie("ESPSESSIONID")
        }
    }).then (response => {
        data = JSON.parse(response.text)
        localStorage.removeItem('smsData')
        localStorage.setItem('smsData', JSON.stringify(data.smss))
        localStorage.removeItem('exceptionData')
        localStorage.setItem('exceptionData', JSON.stringify(data.exceptions))
    }).catch (error => {
        alert("Unable to fetch SMS and exceptions data from hardware: "+ error)
    })
    // response.text = [
    //     "smss" : [
    //         [ id, address, provider, message,   ],
    //         [   ],
    //         [   ],
    //         [   ],
    //     ],
    //     "exceptions" : [
    //         [],
    //         [],
    //         [],
    //         [],
    //     ]
    // ]

    smsTable = $('#smsTable').DataTable( {
        data: json.parse(localStorage.getItem('smsData')),
        dom: 'Bfrtip',
        buttons: [
            'copy', 'csv', 'excel', 'pdf', 'print'
        ],
        columns: [
            { name: "Id"},
            { name: "Address" },
            { name: "Provider" },
            { name: "Message" },
            { name: "ArrivalTime" },
            { name: "Delete" },
        ],
        columnDefs: [
            {
                targets: 3,
                render: function (data, type, row, meta) {
                    return '<input type="button" class="delete" id=n-"' + meta.row + '" value="Name"/>';
                }
            }
        ]
    });

    errorTable = $('#errorTable').DataTable( {
        data: json.parse(localStorage.getItem('errorData')),
        dom: 'Bfrtip',
        buttons: [
            'copy', 'csv', 'excel', 'pdf', 'print'
        ],
        columns: [
            { name: "Id"},
            { name: "message" },
            { name: "trace" },
            { name: "Time" },
            { name: "Delete" },
        ],
        columnDefs: [
            {
                targets: 3,
                render: function (data, type, row, meta) {
                    return '<input type="button" class="delete" id=n-"' + meta.row + '" value="Name"/>';
                }
            }
        ]
    });

    smsTable.on('click', '.delete', function () {
        var id = $(this).attr("id").match(/\d+/)[0];
        var data = smsTable.row( id ).data();
        console.log(data[0]);

        fetch(sms_url + '?id=' + data[0], {
            method: 'DELETE',
            mode: 'cors',
            cache: 'no-cache',
            credentials: 'same-origin',
            headers: {
                'Content-Type': 'application/json',
                'Cookie': "ESPSESSIONID=" + getCookie("ESPSESSIONID")
            }
        }).then( resp => {
            smsDataSet = JSON.parse(localStorage.getItem('smsData'))
            localStorage.removeItem('smsData')
            smsDataSet = smsDataSet.filter(function(value, index, arr) {
                return value[0] != id
            })
            localStorage.setItem('smsData', JSON.stringify(smsDataSet))
            smsTable.row(id).remove().draw( false )
        }).catch(error => {
            alert("Unable to delete SMS data from hardware: "+ error);
        })
    })

    errorTable.on('click', '.delete', function () {
        var id = $(this).attr("id").match(/\d+/)[0];
        var data = smsTable.row( id ).data();
        console.log(data[0]);

        fetch(error_url + '?id=' + data[0], {
            method: 'DELETE',
            mode: 'cors',
            cache: 'no-cache',
            credentials: 'same-origin',
            headers: {
                'Content-Type': 'application/json',
                'Cookie': "ESPSESSIONID=" + getCookie("ESPSESSIONID")
            }
        }).then( resp => {
            errorDataSet = JSON.parse(localStorage.getItem('errorData'))
            localStorage.removeItem('errorData')
            errorDataSet = smsDataSet.filter(function(value, index, arr) {
                return value[0] != id
            })
            localStorage.setItem('errorData', JSON.stringify(errorDataSet))
            errorTable.row(id).remove().draw( false )
        }).catch(error => {
            alert("Unable to delete SMS data from hardware: "+ error);
        })
    })

    errorTable.on( 'click', 'tr', function () {
        if ( $(this).hasClass('selected') ) {
            $(this).removeClass('selected');
        }
        else {
            table.$('tr.selected').removeClass('selected');
            $(this).addClass('selected');
        }
    });
 
    smsTable.on( 'click', 'tr', function () {
        if ( $(this).hasClass('selected') ) {
            $(this).removeClass('selected');
        }
        else {
            table.$('tr.selected').removeClass('selected');
            $(this).addClass('selected');
        }
    });
});