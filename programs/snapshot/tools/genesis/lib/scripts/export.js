// Genesis JSON
const download_genesis_json = ( data )       => export_json( data, 'genesis' )

const export_json = (items, filename) => {
  filename = filename + '.json' || 'export.csv'
  let json = JSON.stringify(items, null, '\t')
  let blob = new Blob([json], { type: 'application/json;charset=utf-8;' })
  if (navigator.msSaveBlob) { // IE 10+
      navigator.msSaveBlob(blob, filename)
  } else {
    let link = document.createElement("a")
    if (link.download !== undefined) { // feature detection
      // Browsers that support HTML5 download attribute
      let url = URL.createObjectURL(blob)
      link.setAttribute("href", url)
      link.setAttribute("download", filename)
      link.style.visibility = 'hidden'
      document.body.appendChild(link)
      link.click()
      document.body.removeChild(link)
    }
  }
}
